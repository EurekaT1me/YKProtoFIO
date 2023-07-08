/** ProtoFIO
 * 参考：https://github.com/yngzMiao/protobuf-parser-tool/tree/master
 */

#include <assert.h>
#include <boost/noncopyable.hpp>
#include <fstream>
#include <string>
#include <vector>
namespace ProtoFIO {
using namespace std;
using boost::noncopyable;
template <typename T>
class ProtoOFstream : public noncopyable {
  private:
    string _name;  // 需要操作的文件名
    ofstream _out; // 写文件流
  public:
    ProtoOFstream(const string &name) : _name(name) {
        _out.open(_name.c_str(), ios::binary | ios::app);
    }
    ~ProtoOFstream() {
        stop();
    }
    // 写入数据，只是写入缓冲，还没有flush到文件中，msg是传入传出参数,pos和len代表偏移和长度
    void write(T *msg, size_t &pos, size_t &len) {
        assert(_out.is_open());
        assert(msg != nullptr);

        // 使用proto提供的方法计算字节数
        auto byteLen = msg->ByteSizeLong();
        // 将字节数按4字节大端字节序存放在首位
        char temp[4];
        temp[3] = byteLen & 0x00FF;
        temp[2] = (byteLen >> 8) & 0x00FF;
        temp[1] = (byteLen >> 16) & 0x00FF;
        temp[0] = (byteLen >> 24) & 0x00FF;
        _out.write(temp, sizeof(temp));

        // 跟新pos和len
        pos = _out.tellp();
        len = byteLen;

        // 判断文件是否成功打开并且能写入，用于判断写入temp是否正常
        assert(_out.good());

        // 序列化写入实际的数据
        assert(byteLen > 0 && msg->SerializeToOstream(&_out) == true);
    }
    // 写入数据
    void write(T *msg) {
        size_t pos, len;
        write(msg, pos, len);
    }
    // 关闭文件
    void stop() {
        if (_out.is_open()) {
            _out.close();
        }
    }
    // flush文件
    void flush() {
        _out.flush();
    }
};

template <typename T>
class ProtoIFstream : public noncopyable {
    // proto结构数据的信息
    typedef struct protoInfo {
        size_t prtPos; // 文件指针偏移，用来指出从哪儿个位置开始读
        size_t prtLen; // 待读取的数据长度
    } protoInfo_t;

  private:
    string _name;                    // 需要操作的文件名
    ifstream _in;                    // 读文件流
    vector<protoInfo_t> _protoInfos; // 文件中每条数据的偏移和长度
    vector<char> _protoData;         // 存放proto字节流

  public:
    ProtoIFstream(const string &name) : _name(name) {
        // 打开文件
        _in.open(_name.c_str(), ios::binary);
        init();
    }
    ~ProtoIFstream() { stop(); }
    // 关闭文件
    void stop() {
        if (_in.is_open()) {
            _in.close();
        }
    }
    // 读某一条数据，msg是传入传出参数,pos表示某一条索引
    void read(T *msg, int pos) {
        // 验证pos合法性
        assert(pos >= 0 && pos < count());

        // 获取pos对应的数据info
        protoInfo frameInfo = _protoInfos[pos];
        // 根据frameInfo进行数据读取
        _in.seekg((int)frameInfo.prtPos);
        _in.read(&_protoData[0], frameInfo.prtLen);
        // 调用protobuf提供的ParseFromArray,该函数从字符数组中反序列化，注意下面需要转化成unsigned char类型指针
        assert(msg->ParseFromArray((uint8_t *)(&_protoData[0]), frameInfo.prtLen) == true);
    }

    // 返回数据条数
    size_t count() const { return _protoInfos.size(); }

    // 添加一条protoInfo记录,不负责检查文件中是否存在，主要用在同时读写时候的更新
    void add(size_t pos, size_t len) {
        // 将pos和len存储到结构体
        protoInfo_t protoInfo;
        protoInfo.prtLen = len;
        protoInfo.prtPos = pos;
        // 添加条数据信息
        _protoInfos.emplace_back(protoInfo);
        // 更新protodata数组容量
        if (_protoData.capacity() < len) {
            _protoData.reserve(len);
        }
    }

  private:
    // 遍历整个文件获得每条数据的偏移和长度，存放成一个数组
    void init() {

        assert(_in.is_open());
        _protoInfos.clear();

        // 获得文件长度
        size_t fileSize = getFileSize();
        // 用于记录文件中proto数据数目
        int protoNum = 0;
        // 当前光标在文件中的位置
        size_t curPos = 0;
        // 存放protoInfo数据
        protoInfo_t protoInfo;
        // 记录一个proto数据的最大长度
        size_t maxProtoDataLen = 0;
        while (curPos < fileSize) {
            size_t protoDataLen = 0;
            // 一个proto数据的前4个字节存放了数据的长度
            char temp[4];
            _in.read(temp, 4);
            protoDataLen = (temp[0] << 24) & 0xFF000000;
            protoDataLen |= (temp[1] << 16) & 0x00FF0000;
            protoDataLen |= (temp[2] << 8) & 0x0000FF00;
            protoDataLen |= (temp[3]) & 0x000000FF;
            // 偏移protoDataLen长度，注意在seekg运行前_in.cur已经移动了4个字节
            _in.seekg(protoDataLen, _in.cur);

            // 存放这条数据的偏移和长度
            protoInfo.prtPos = curPos + 4;
            protoInfo.prtLen = protoDataLen;

            // 将数据信息写入内存中的数组用于索引
            _protoInfos.emplace_back(protoInfo);

            // 移动curPos当下一条数据
            curPos = curPos + 4 + protoDataLen;
            // 更新数据的最大长度
            maxProtoDataLen = max(maxProtoDataLen, protoDataLen);
        }
        printf("%lu\n", count());
        // 更新_protoData容量
        if (_protoData.capacity() < maxProtoDataLen) {
            _protoData.reserve(maxProtoDataLen);
        }
    }

    // 计算文件长度
    size_t getFileSize() {
        // 获得当前文件中指针的位置
        size_t cur = _in.tellg();
        // 定位到文件末尾，函数的意思是相对end位置偏移0个字节
        _in.seekg(0, _in.end);
        // 获取文件末尾位置,即文件长度
        size_t size = _in.tellg();
        // 将文件重新定位
        _in.seekg(cur);
        return size;
    }
};

} // namespace ProtoFIO
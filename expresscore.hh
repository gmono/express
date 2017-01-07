#ifndef EXPRESSCORE_HH
#define EXPRESSCORE_HH
#include <stack>
#include <vector>
#include  <functional>
#include <unordered_map>
#include <deque>
#include <set>
using namespace std;
#define umap unordered_map


typedef function<int(const vector<int> &)> ExpFunc;//标准函数定义
typedef function<int(const int,const int)> SymbolFunc;//符号函数定义
//表达式运算类
class ExpressCore
{
public:
    ExpressCore();


private:

    enum class WordType{Number,Sym,None};

    struct Symbol
    {
        //    string symsign="";//这是调试用的方便识别符号
        Symbol(){}
        Symbol(int level,SymbolFunc &func);
        int level=0;//优先级0为最低级
        SymbolFunc func;
    };
    struct Word
    {
        WordType type=WordType::None;
        struct
        {
            int number;
            Symbol sym;
        }data;
    };
    //这里建立一个容器可访问的stack适配器
    template<typename _Tp, typename _Sequence = deque<_Tp> >
    struct istack:public stack<_Tp,_Sequence>
    {
        _Sequence *contptr;
        istack();
    };
    umap<string,Symbol *> symmap; //符号函数映射表 符号->func(a,b) 用于查询运算符的存在性和取得运算符函数 运算符不能为逗号和括号
    umap<string,ExpFunc> exmap;//表达式函数表

    //原则 任何一个读取函数读取后 一切属于读取范围内的内容都被跳过
    Word readNumber(int &ptr,string &text);
    Word readSymbol(int &ptr,string &text);
    Word readFunction(int &ptr,string &text);
public:
    //这个函数会自动跳过空格 但是如果设置成空格结束 也可以
    //这个函数是计算函数 读取一个表达式返回一个值
    int readExpression(int &ptr,string &text,string signs);
    //下面是注册函数

    //注册一个运算符
    //返回值表示是否已经存在（即这次操作是否属于替换操作）
    bool registSymbol(const string &name,int level,const SymbolFunc &func);
    //注册一个函数
    void registFunction(const string &name,ExpFunc func);
};

#endif // EXPRESSCORE_HH

#include "expresscore.hh"

ExpressCore::ExpressCore()
{
}

ExpressCore::Word ExpressCore::readNumber(int &ptr, string &text)
{
    //这里有个基本认识即string的end就是\0
    int sum=0;
    for(char c=text[ptr++];c<='9'&&c>='0';c=text[ptr++])
    {
        sum=sum*10+(c-'0');
    }
    ptr--;//调整 由于循环完时会让ptr++ 因此会在结束符后面一位 调整到前一位
    Word ret;
    ret.type=WordType::Number;
    ret.data.number=sum;
    return ret;
}

ExpressCore::Word ExpressCore::readSymbol(int &ptr, string &text)
{
    //运算符 遇到任何字母或数字时停止
    string symbuf;
    for(char c=text[ptr++];!isalnum(c);c=text[ptr++])
    {
        symbuf.push_back(c);
    }
    ptr--;//调节
    auto sptr=symmap.find(symbuf);
    if(sptr!=symmap.end())
    {
        Word ret;
        ret.type=WordType::Sym;
        ret.data.sym=*(sptr->second);
        return ret;
    }
    throw "运算符错误";
}

ExpressCore::Word ExpressCore::readFunction(int &ptr, string &text)
{
    int readExpression(int &ptr,string &text,string signs);
    //函数遇到字母时进入函数读取模式
    string namebuf;//函数名存储
    for(char c=text[ptr++];isalnum(c);c=text[ptr++])
    {
        namebuf.push_back(c);
    }
    ptr--;//调整
    if(text[ptr]!='(')
    {
        string err;
        err="期望得到'(',实际得到'";
        err.push_back(text[ptr]);
        err.push_back('\'');
        throw err;
    }
    auto sptr=exmap.find(namebuf);
    if(sptr==exmap.end()) throw "没有这个函数";
    ExpFunc fun=sptr->second;
    //读取参数表 参数表以逗号分割 每一个参数都是一个表达式
    vector<int> pars;//参数表
    while(ptr<text.length())//这里保证其不会越界
    {
        int p=readExpression(ptr,text,",)");//以逗号和反括号结尾 结尾时ptr指向逗号
        pars.push_back(p);
        if(text[ptr]==',') continue;
        if(text[ptr]==')') break;//遇到反括号就跳出
        else throw "内部错误！";//这里理应是上面两个字符 如果不是就是内部错误了
    }
    //调用函数得到结果
    int num=fun(pars);
    Word ret;
    ret.type=WordType::Number;
    ret.data.number=num;
    return ret;
}

int ExpressCore::readExpression(int &ptr, string &text, string signs)
{

    //signs中的任意字符为结束标志 同时末尾自动结束
    stack<Word> stack1; //符号栈
    istack<Word> stack2; //操作数栈
    Word ls;//最低优先级运算符
    ls.type=WordType::Sym;
    ls.data.sym=Symbol(); //默认新的symbol 初始优先级为最低0
    stack1.push(ls);
    //构造查询set
    set<char> query;
    for(int i=0;i<signs.length();++i) query.insert(signs[i]);
    //初始化完成
    for(;;)
    {
        if(ptr>=text.length()) break;//保证不越界
        char c=text[ptr];
        if(query.find(c)!=query.end()) break;//遇到结束标记 结束
        if(c==' ') continue;//跳过空格
        if(c=='(')
        {
            int p=readExpression(ptr,text,")");
            Word temp;
            temp.type=WordType::Number;
            temp.data.number=p;
            stack2.push(temp);
            //此时ptr指向反括号
            ptr++;//跳过反括号
        }
        else if(isdigit(c))
        {
            //数字
            Word temp=readNumber(ptr,text);
            stack2.push(temp);
        }
        else if(isalpha(c))
        {
            //字母
            //当函数处理
            Word temp=readFunction(ptr,text);
            stack2.push(temp);
        }
        else
        {
            //当操作符处理
            Word temp=readSymbol(ptr,text);
            int nlevel=temp.data.sym.level;
            Word top=stack1.top();
            int olevel=top.data.sym.level;
            for(;;)
            {
                if(nlevel>olevel)
                {
                    stack1.push(temp);
                    break;
                }
                else
                {
                    //将s1的顶运算符弹出送入s2栈
                    stack2.push(stack1.top());
                    stack1.pop();
                    //更新olevel
                    top=stack1.top();
                    olevel=top.data.sym.level;
                }
            }
        }

    }
    //处理完成，最后一步把stack1中的运算符出到stack2中
    for(Word t=stack1.top();t.data.sym.level!=0;t=stack1.top())
    {
        stack2.push(t);
        stack1.pop();
    }
    //构造完成
    deque<Word> &cont=*(stack2.contptr);//stack从低向高生长 因此直接zheng正序遍历
    //计算开始
    stack<int> nums;//数字计算栈
    for(auto i=cont.begin();i!=cont.end();++i)
    {
        Word w=*i;
        if(w.type==WordType::Number) nums.push(w.data.number);
        else if(w.type==WordType::Sym)
        {
            //运算符 取出两个计算后再压入
            //例如 1+2+这样的表达式就会出现 12++这样的逆波兰式 这样就会出现取数错误
            try
            {
                int a=nums.top();
                nums.pop();
                int b=nums.top();
                nums.pop();
                int sum=w.data.sym.func(a,b);
                nums.push(sum);
            }
            catch(...)
            {
                throw "取数错误！数字个数不正确";
            }
        }
        else throw "内部错误";//无故出现None型对象 内部错误
    }
    //得到结果
    int ret=nums.top();
    return ret;
}

bool ExpressCore::registSymbol(const string &name, int level, const SymbolFunc &func)
{
    bool isexist=symmap.find(name)!=symmap.end()? true:false;//如果存在就返回true
    if(isexist)
    {
        Symbol *old=symmap[name];
        symmap.erase(name);
        delete old;
        //释放原来的
    }
    Symbol *symbol=new Symbol(level,func);
    this->symmap[name]=symbol;
    return isexist;//这里是返回true代表替换 相反的话可以加个取反
}

void ExpressCore::registFunction(const string &name, ExpFunc func)
{
    bool isexist=exmap.find(name)!=exmap.end()? true:false;//如果存在就返回true
//    if(isexist) exmap.erase(name);//擦除
    //由于无需释放，所以直接替换 应该没事
    this->exmap[name]=func;//允许替换
}

ExpressCore::Symbol::Symbol(int level, ExpressCore::SymbolFunc &func)
{
    this->level=level;
    this->func=func;
}

ExpressCore::istack::istack(){this->contptr=&this->c;}

#include<bits/stdc++.h>
using namespace std;

struct OpCode{
    int format;
    string opcode;
    OpCode(): format(0), opcode("") {}
    OpCode(int a, string b): format(a), opcode(b) {}
};
struct Inf{
    int addr;
    string op[3];
    string code;
    Inf(int nowaddr, string s1, string s2, string s3){
        addr  = nowaddr;
        op[0] = s1;
        op[1] = s2;
        op[2] = s3;
        code  = "";
    }
};
map<string,OpCode>optab;
map<string,int> symtable;
map<string,int> reg;
vector<Inf>objfile;
vector<int>mstack;


int nowaddr = 0;
int staraddr;
int endaddr;
string ProgramName;
int b_register = 0;

void init(){
    ifstream ins("op_code.txt");
    if(!ins){
        cout<<"op_code input error\n";
    }
    string name; 
    int format; 
    string op;
    string bir;
    while(ins>>name>>format>>op>>bir){
        optab[name] = OpCode(format, op);
    }
    ins.close();
    cout<<"---- op_table ----\n";
    for(map<string,OpCode>::iterator it=optab.begin();it!=optab.end();it++){
        cout<<it->first<<"\t"<<it->second.format<<"\t"<<it->second.opcode<<"\n";
    }
    reg["A"]  = 0;
    reg["X"]  = 1;
    reg["L"]  = 2;
    reg["PC"] = 8;
    reg["SW"] = 9;
    reg["B"]  = 3;
    reg["S"]  = 4;
    reg["T"]  = 5;
    reg["F"]  = 6;
}


string int_to_hex(int bi,int value){
    vector<int>v;
    v.clear();
    int ba=1;
    //bool bo = false;
    if(value<0){
        
        for(int i=0;i<bi*4;i++){
            ba*=2;
        }
        value=ba+value;
    }
    while(value!=0){
        v.push_back(value%16);
        value = value/16;
    }
    int n = v.size();
    string s="";
    if(n>bi){
        cout<<"RE\n";
    }
    while(n<bi){
        s+="0";
        n++;
    }
    for(int i=v.size()-1;i>=0;i--){
        s+=(v[i]<10)?(char)(v[i]+'0'):(char)(v[i]-10+'A');
    }
    
    return s;
}
int hex_to_int(string s){
    int now = 0;
    int ba = 1;
    for(int i=s.length()-1;i>=0;i--){
        now += (ba* ( s[i]>='A'&&s[i]<='F'?s[i]-'A'+10:s[i]-'0'));
        ba *= 16;
    }
    return now;
}


int main(int argc, char *argv[]) {
    init();
    string textbook;
    if(argc<2){
        textbook = "textbooksicxe.asm";
    }
    else if(argc == 2){
        textbook = argv[1];
    }
    else{
        cout<<"error"<<endl;
        return 0;
    }

//------------------pass 1-----------------------------
    ifstream file(textbook);
    string line;
    while ( getline(file, line) ){
        string s1, s2, s3;
        string s;
        if(line[0]=='.')continue;
        
        vector<string>arr;
        arr.clear();

        stringstream ss;
        //cout<<"line:"<<line<<endl;
        ss<<line;
        while(ss>>s){
            arr.push_back(s);
        }
        //cout<<"arr.szie():"<<arr.size()<<endl;
        switch (arr.size()){
            case 1:
                s1 = "*";
                s2 = arr[0];
                s3 = "*";
                break;
            case 2:
                s1 = "*";
                s2 = arr[0];
                s3 = arr[1];
                break;
            case 3:
                s1 = arr[0];
                s2 = arr[1];
                s3 = arr[2];
                break;
            default:
                cout<<"input string error.\n";
                break;
        }
        //cout<<s2<<"  nowaddr:"<<nowaddr<<endl;
        objfile.push_back(Inf(nowaddr, s1, s2, s3));
        if(s1 != "*"){
            symtable[s1] = nowaddr;
        }
        
        if(s2 == "START"){
            nowaddr = hex_to_int(s3);
            staraddr = nowaddr;
            //cout<<"start stoi:"<<stoi(s3)<<" \n";
            ProgramName = s1;
        }
        else if(s2 == "RESW"){
            nowaddr += stoi(s3) * 3;//
        }
        else if(s2 == "RESB"){
            nowaddr += stoi(s3);//
        }
        else if(s2 == "WORD"){
            nowaddr += 3; 
        }
        else if(s2 == "BYTE"){
            if(s[0]=='X')       nowaddr += ((s.length()-3)/2)+((s.length()-3)%2);
            else if(s[0]=='C')  nowaddr += s.length()-3;//dis C''
            else {
                cout<<"s is :"<<s<<"  ||  bytesizs error.\n";
            }
        }
        else if(s2 == "END"){
            endaddr = nowaddr - staraddr;
        }
        else if(s2[0] == '+')
            nowaddr += 4;//format 4 (4byte)
        else if(optab.find(s2)!=optab.end()){
            nowaddr += optab[s2].format;
        }
    }
    file.close();
    cout<<"\n--------- Pass 1: ---------"<<endl;
    for(Inf p : objfile){
        cout<<int_to_hex(4,p.addr)<<"\t"<<p.op[0]<<"\t"<<p.op[1]<<"\t"<<p.op[2]<<endl;
    }
    cout<<"\n";
    //------------------pass 2-----------------------------
    string word;
    for(int i=0;i<objfile.size();i++){
        string s="";
        switch (optab[objfile[i].op[1]].format){
            case 1://format 1 
                s=optab[objfile[i].op[1]].opcode;//8bit opcode
                break;
            case 2://format 2
                s=optab[objfile[i].op[1]].opcode;//8bit opcode
                int reg1;
                int reg2;
                reg2 = reg["A"];
                word = "";
                int j;
                for(j=0;j<objfile[i].op[2].size()&&objfile[i].op[2][j]!=',';j++){
                    word+=objfile[i].op[2][j];
                }
                reg1 = reg[word];
                if(objfile[i].op[2][j]==','){
                    while(objfile[i].op[2][j]==','||objfile[i].op[2][j]==32)j++;
                    word = "";
                    for(;j<objfile[i].op[2].size();j++){
                        word+=objfile[i].op[2][j];
                    }
                    reg2 = reg[word];
                }
                s+=int_to_hex(1,reg1);
                s+=int_to_hex(1,reg2);
                break;
            case 3://format 3
                int opnum;
                opnum = hex_to_int(optab[objfile[i].op[1]].opcode);
                int nn=1,ii=1,xx=0,bb=0,pp=0,ee=0;
                int disp = 0;
                if(objfile[i].op[2][objfile[i].op[2].size()-1]=='X'){
                    xx=1;
                }
                if(objfile[i].op[2][0]=='@'){//indirect
                    nn=1;ii=0;
                }
                else if(objfile[i].op[2][0]=='#'){//immediate
                    nn=0;ii=1;    
                }
                word = "";
                for(int j=0;j<objfile[i].op[2].size()&&objfile[i].op[2][j]!=',';j++){
                    if(objfile[i].op[2][j]=='@'||objfile[i].op[2][j]=='#')continue;
                    word+=objfile[i].op[2][j];
                }
                if(objfile[i].op[2][0]=='*'){
                    disp=0;
                }
                else if(word[0]>='0'&&word[0]<='9'){
                    bb=0;pp=0;
                    disp = word[0]-'0';
                }
                else{
                    disp = symtable[word] - objfile[i+1].addr;
                    if(disp <=2047 && disp >= -2048){
                        bb=0;pp=1;
                    }
                    else{
                        bb=1;pp=0;
                    }
                }                
                if(objfile[i].op[1]=="LDB"){
                    b_register =  symtable[word];
                }
                //cout<<word<<" "<<symtable[word]<<" "<<objfile[i].addr<<"   : "<<disp<<" ";
                //cout<<objfile[i].op[2]<<" ";
                //cout<<xx<<" "<<bb<<" "<<pp<<" "<<ee<<"  ?? "<<int_to_hex(3,disp)<<endl;
                //cout<<word<<" "<<symtable[word]-b_register<<" | "<<symtable[word]<<" "<<b_register<<endl;
                s+=int_to_hex(2,(opnum+2*nn+ii));
                s+=int_to_hex(1,xx*8+bb*4+pp*2+ee);
                if(disp <=2047 && disp >= -2048){
                    s+=int_to_hex(3,disp);
                }
                else{
                    string res=int_to_hex(4,symtable[word]-b_register);
                    s+=res[1];
                    s+=res[2];
                    s+=res[3];
                }
                break;
        }
        if(objfile[i].op[1][0]=='+'){//format 4
            int nn=1,ii=1,xx=0,bb=0,pp=0,ee=1;
            string newstr ="";
            for(int k=1;k<objfile[i].op[1].size();k++)newstr+=objfile[i].op[1][k];
            //cout<<"newstr:"<<newstr<<endl;
            int opnum=hex_to_int(optab[newstr].opcode);
            if(objfile[i].op[2][objfile[i].op[2].size()-1]=='X'){//index
                xx=1;
            }
            if(objfile[i].op[2][0]=='@'){//indirect
                nn=1;ii=0;
            }
            if(objfile[i].op[2][0]=='#'){//immediate
                nn=0;ii=1;
            }
            
            s+=int_to_hex(2,(opnum+2*nn+ii));
            s+=int_to_hex(1,xx*8+bb*4+pp*2+ee);
            if(objfile[i].op[2][0]=='#'){//immediate
                int sval = 0;
                for(int k=objfile[i].op[2].size()-1,kk=1;k>=1;k--,kk*=10)sval+=((objfile[i].op[2][k]-'0')*kk);
                //cout<<sval<<endl;
                string res=int_to_hex(6,sval);
                //cout<<"res:"<<res<<endl;
                for(int k=1;k<6;k++)s+=res[k];
            }
            else if(nn==1&&ii==1){
                string news=int_to_hex(6,symtable[objfile[i].op[2]]);
                mstack.push_back(objfile[i].addr+1);
                for(int k=1;k<6;k++)s+=news[k];
            }
        }
        if(objfile[i].op[1]=="BYTE"){
            if(objfile[i].op[2][0]=='X'){
                for(int j=2;j<objfile[i].op[2].size()-1;j++){
                    s+=objfile[i].op[2][j];
                }
            }
            else if(objfile[i].op[2][0]=='C'){
                for(int j=2;j<objfile[i].op[2].size()-1;j++){
                    string res=int_to_hex(3,objfile[i].op[2][j]);
                    s+=res[1];s+=res[2];
                }
            }
        }
        objfile[i].code=s;
    }
    cout<<"-------------- Pass 2: -------------"<<endl;
    printf("%4s\t\t%24s\t\t%8s\n", "LOC", "SOURCE STATEMENT", "OBJECT CODE");
    for( auto p : objfile){
        printf("%4s\t%8s\t%8s\t%8s\t%8s\n", int_to_hex(4,p.addr).c_str(), p.op[0].c_str(), p.op[1].c_str(), p.op[2].c_str(), p.code.c_str());
    }
    cout<<"\n";
    //---------output.obj----------------------------------
    ofstream outputfile;
    outputfile.open("textbook.obj");
    
    //header record
    outputfile<<"H";
    outputfile<<ProgramName;
    for(int i=ProgramName.length();i<6;i++){
        outputfile<<" ";
    }
    string s = int_to_hex(7,staraddr);
    for(int k=1;k<7;k++)outputfile<<s[k];
    s = int_to_hex(7,endaddr-staraddr);
    for(int k=1;k<7;k++)outputfile<<s[k];
    outputfile<<"\n";

    //Text record
    string news = "";
    int nowstar=staraddr;
    //cout<<"nowstar:"<<nowstar<<endl;
    //60
    for(int i=0;i<objfile.size()-1;i++){
        //cout<<"objfile[i+1].addr:"<<objfile[i+1].addr<<" nowstar:"<<nowstar<<" dis:"<<objfile[i+1].addr-nowstar<<endl;
        if(objfile[i+1].addr-nowstar<=30||objfile[i].code.length()==0){
            if(news==""){
                //cout<<"go there"<<endl;
                news="T";
                string res = int_to_hex(7,nowstar);
                for(int k=1;k<7;k++)news+=res[k];
                news+="XX";//7,8
                //cout<<"----"<<news<<endl;
            }
            if(objfile[i].code.length()==0){
                //for(int i=0;i<objfile[i+1].addr-objfile[i].addr;i++){
                //    news+=" ";
                //}
            }
            else{
                news+=objfile[i].code;
            }
        }
        else{
            //cout<<" go in here"<<endl;
            string rs=int_to_hex(3,(news.length()-9)/2);
            news[7]=rs[1];
            news[8]=rs[2];
            //cout<<"rs:"<<rs<<endl;
            //cout<<news.length()<<endl;
            outputfile<<news;
            outputfile<<"\n";
            news="";
            nowstar=objfile[i].addr;
            i--;
        }
    }
    if(news.length()!=0){
        string rs=int_to_hex(3,(news.length()-9)/2);
        news[7]=rs[1];
        news[8]=rs[2];
        //cout<<news.length()<<endl;
        //cout<<"rs:"<<rs<<endl;
        outputfile<<news;
        outputfile<<"\n";
    }
    //M record
    for(int i=0;i<mstack.size();i++){
        outputfile<<"M";
        string res = int_to_hex(7,mstack[i]);
        for(int k=1;k<7;k++)outputfile<<res[k];
        outputfile<<"05\n";
    }

    //End record
    outputfile<<"E";
    string res = int_to_hex(7,staraddr);
    for(int k=1;k<7;k++)outputfile<<res[k];
    outputfile<<"\n";
    outputfile.close();
}
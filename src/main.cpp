#include "image_common.h"
using Feature=std::array<double,72>;
struct Sample{Image image;int label;Feature hist;};
std::vector<Sample> database,queries;std::vector<std::string> labels;
std::vector<std::pair<double,int>> results;Image query;int queryLabel=-1;bool euclidean=false;
Feature feature(const Image& im){
 if(im.rgb.empty()||im.rgb.size()!=size_t(im.w)*im.h*3)throw std::runtime_error("Invalid RGB image");
 Feature f{};
 for(size_t i=0;i<im.rgb.size();i+=3){double r=im.rgb[i]/255.,g=im.rgb[i+1]/255.,b=im.rgb[i+2]/255.;double hi=std::max({r,g,b}),lo=std::min({r,g,b}),d=hi-lo,h=0,s=hi==0?0:d/hi;
 if(d>0){if(hi==r)h=60*std::fmod((g-b)/d,6);else if(hi==g)h=60*((b-r)/d+2);else h=60*((r-g)/d+4);if(h<0)h+=360;}
 int H=h<20||h>=316?0:h<40?1:h<75?2:h<155?3:h<190?4:h<270?5:h<295?6:7;
 int S=s<0.2?0:s<0.7?1:2,V=hi<0.2?0:hi<0.7?1:2;
 ++f[9*H+3*S+V];}
 for(auto& v:f)v/=double(im.w)*im.h;
 return f;
}
std::vector<Sample> readSplit(const fs::path& path,size_t count){
 std::ifstream in(path,std::ios::binary);if(!in||fs::file_size(path)!=count*3074)throw std::runtime_error("Missing or invalid CIFAR-100 binary split");
 std::vector<Sample> out;out.reserve(count);std::array<unsigned char,3074> row;
 for(size_t n=0;n<count;++n){in.read((char*)row.data(),row.size());if(!in||row[0]>=20||row[1]>=100)throw std::runtime_error("Invalid CIFAR record");Sample s;s.label=row[1];s.image={32,32,std::vector<unsigned char>(3072)};for(int p=0;p<1024;++p)for(int c=0;c<3;++c)s.image.rgb[p*3+c]=row[2+c*1024+p];s.hist=feature(s.image);out.push_back(std::move(s));}return out;
}
void load(){if(!database.empty())return;auto root=baseDir()/L"数据集/cifar-100-binary";std::vector<std::string> names;std::ifstream meta(root/L"fine_label_names.txt");std::string line;while(std::getline(meta,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();names.push_back(line);}if(names.size()!=100)throw std::runtime_error("CIFAR-100 label names not found");auto train=readSplit(root/L"train.bin",50000);auto test=readSplit(root/L"test.bin",10000);labels=std::move(names);database=std::move(train);queries=std::move(test);}
std::vector<std::pair<double,int>> search(const Feature& q,bool distance){std::vector<std::pair<double,int>> v;v.reserve(database.size());for(size_t i=0;i<database.size();++i){double score=0;for(int j=0;j<72;++j){double a=database[i].hist[j];score+=distance?(q[j]-a)*(q[j]-a):std::min(q[j],a);}v.emplace_back(distance?std::sqrt(score):score,int(i));}std::partial_sort(v.begin(),v.begin()+10,v.end(),[distance](auto a,auto b){if(a.first==b.first)return a.second<b.second;return distance?a.first<b.first:a.first>b.first;});v.resize(10);return v;}
std::wstring wide(const std::string& s){return std::wstring(s.begin(),s.end());}
void saveResults(const fs::path& dir,const Image& im,int label,const std::vector<std::pair<double,int>>& v){fs::create_directories(dir);saveBmp(im,dir/L"query.bmp");std::ofstream csv(dir/L"Top10.csv");csv<<"rank,train_index,label,score,same_class\n";for(size_t k=0;k<v.size();++k){auto [score,index]=v[k];std::string filename="rank_"+std::to_string(k+1)+".bmp";saveBmp(database[index].image,dir/fs::path(filename));csv<<k+1<<","<<index<<","<<labels[database[index].label]<<","<<score<<","<<(label<0?-1:int(label==database[index].label))<<"\n";}if(!csv)throw std::runtime_error("Cannot write result CSV");}
void runSearch(HWND w){load();euclidean=SendDlgItemMessageW(w,12,CB_GETCURSEL,0,0)==1;results=search(feature(query),euclidean);int hits=0;for(auto p:results)hits+=database[p.second].label==queryLabel;std::wstring s=L"检索库 50000 幅训练图像    Top-10    "+std::wstring(euclidean?L"欧氏距离越小越相似":L"直方图相交值越大越相似");if(queryLabel>=0)s+=L"    同类 "+std::to_wstring(hits)+L" / 10";SetDlgItemTextW(w,20,s.c_str());InvalidateRect(w,nullptr,TRUE);}
LRESULT CALLBACK proc(HWND w,UINT m,WPARAM wp,LPARAM lp){try{switch(m){case WM_CREATE:
 ctl(w,L"STATIC",L"测试集编号 0–9999",0,20,22,140,24);ctl(w,L"EDIT",L"0",10,165,18,65,28,WS_BORDER|ES_NUMBER);ctl(w,L"BUTTON",L"按编号检索",1,245,18,120,30);ctl(w,L"BUTTON",L"打开查询图像",2,380,18,130,30);
 ctl(w,L"COMBOBOX",L"",12,525,18,210,120,CBS_DROPDOWNLIST);SendDlgItemMessageW(w,12,CB_ADDSTRING,0,(LPARAM)L"HSV 直方图相交");SendDlgItemMessageW(w,12,CB_ADDSTRING,0,(LPARAM)L"HSV 欧氏距离");SendDlgItemMessageW(w,12,CB_SETCURSEL,0,0);ctl(w,L"BUTTON",L"导出检索结果",3,750,18,140,30);
 ctl(w,L"STATIC",L"输入测试集编号后点击检索，首次加载需数秒。",20,20,65,1040,30);return 0;
 case WM_COMMAND:switch(LOWORD(wp)){case 1:{int index=number(w,10,0,9999);SetCursor(LoadCursor(nullptr,IDC_WAIT));load();query=queries[index].image;queryLabel=queries[index].label;runSearch(w);SetCursor(LoadCursor(nullptr,IDC_ARROW));break;}
 case 2:{auto p=choose(w);if(p.empty())break;Image im=loadImage(p);query=std::move(im);queryLabel=-1;runSearch(w);break;}
 case 3:if(results.empty())throw std::runtime_error("Run a query first");saveResults(baseDir()/L"实验结果/界面导出",query,queryLabel,results);SetDlgItemTextW(w,20,L"已导出至 实验结果\\界面导出，包含 query.bmp、Top10.csv 及十幅结果图像。");break;}return 0;
 case WM_PAINT:{PAINTSTRUCT p;auto dc=BeginPaint(w,&p);if(!query.rgb.empty()){draw(dc,query,20,130,150,150);std::wstring text=L"查询图像";TextOutW(dc,20,105,text.c_str(),int(text.size()));if(queryLabel>=0){text=wide(labels[queryLabel]);TextOutW(dc,20,290,text.c_str(),int(text.size()));}}for(size_t k=0;k<results.size();++k){int x=205+int(k%5)*172,y=125+int(k/5)*220;auto [score,index]=results[k];draw(dc,database[index].image,x,y,145,145);std::wostringstream s;s.precision(4);s<<k+1<<L"  "<<wide(labels[database[index].label]);auto text=s.str();TextOutW(dc,x,y+150,text.c_str(),int(text.size()));s.str(L"");s<<L"score="<<score;text=s.str();TextOutW(dc,x,y+172,text.c_str(),int(text.size()));}EndPaint(w,&p);return 0;}
 case WM_DESTROY:PostQuitMessage(0);return 0;}}catch(const std::exception& e){SetCursor(LoadCursor(nullptr,IDC_ARROW));error(w,e);}return DefWindowProcW(w,m,wp,lp);}
void selfTest(){
 Image red{1,1,{255,0,0}},gray{1,1,{128,128,128}};auto f=feature(red),g=feature(gray);if(f[8]!=1||g[1]!=1)throw std::runtime_error("HSV bin self test failed");
 load();auto dir=baseDir()/L"实验结果";fs::create_directories(dir);std::ofstream summary(dir/L"验证汇总.csv");summary<<"query_index,label,method,top1_index,top1_score,same_class_at_10\n";
 for(int id:{0,1,2,42,100})for(bool method:{false,true}){auto v=search(queries[id].hist,method);int hits=0;for(auto p:v)hits+=database[p.second].label==queries[id].label;summary<<id<<","<<labels[queries[id].label]<<","<<(method?"euclidean":"intersection")<<","<<v[0].second<<","<<v[0].first<<","<<hits<<"\n";saveResults(dir/fs::path("query_"+std::to_string(id)+(method?"_euclidean":"_intersection")),queries[id].image,queries[id].label,v);}
 auto v=search(database[0].hist,false);if(std::abs(v[0].first-1)>1e-10)throw std::runtime_error("Self similarity failed");
 double sum=0;for(auto x:database[0].hist)sum+=x;if(std::abs(sum-1)>1e-10)throw std::runtime_error("Histogram normalization failed");
 double correct=0;for(int id=0;id<100;++id){auto top=search(queries[id].hist,false);for(auto p:top)correct+=database[p.second].label==queries[id].label;}
 std::ofstream log(dir/L"自检.txt");log<<"PASS: red bin, gray bin, normalized histogram, self similarity.\ntrain=50000\ntest=10000\nfeatures=72\nmetric=intersection\nevaluation_queries=test[0:100]\nmean_precision_at_10="<<correct/1000<<"\n";if(!log||!summary)throw std::runtime_error("Cannot write validation log");
}
int WINAPI wWinMain(HINSTANCE h,HINSTANCE,LPWSTR,int){ULONG_PTR token;Gdiplus::GdiplusStartupInput g;if(Gdiplus::GdiplusStartup(&token,&g,nullptr)!=Gdiplus::Ok)return 1;int ret=0;try{if(std::wstring(GetCommandLineW()).find(L"--self-test")!=std::wstring::npos)selfTest();else ret=runWindow(h,L"ColorRetrievalLab",L"实验3.2 基于颜色的图像检索 CIFAR-100",proc,1100,650);}catch(const std::exception& e){fs::create_directories(baseDir()/L"实验结果");std::ofstream(baseDir()/L"实验结果/错误.txt")<<e.what();ret=1;}Gdiplus::GdiplusShutdown(token);return ret;}

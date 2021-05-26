#include<stdio.h>
#include<math.h>
#include<string.h>
#define N 100   //data_num
#define M 100   //feature_num
#define visual_switch 1 //1-输出中文 0-输出数字

int ans_tree[100][100];
int ans_tree_line;

int feature_num;
int different_tag_num;
int source_data[N][M];
int source_tag[N];

char feature_buffer[][100]={"色泽","根蒂","敲声","纹理","脐部","触感"};
char tag_buffer[][100] = {"坏瓜","好瓜"};
char feature_list_buffer[][6][100]={{"青绿","乌黑","浅白"},{"蜷缩","稍蜷","硬挺"},
                                    {"浊响","沉闷","清脆"},{"清晰","稍糊","模糊"},
                                    {"凹陷","稍凹","平坦"},{"硬滑","软粘"}};

// log2（）函数实现
double log2(double x){
    return log(x)/log(2);
}

// 计算-p*log2(p)
double info_part(double x){
    // printf("%lf\n",x);
    if (x==0) 
        return 0;
    else 
        return 0-x*log2(x);
}

// 检查当前特征中有多少类 或 检查tag有多少类
int find_different_feature_num(int data[][M],int tag[N], int feature_id){
    int temp[N];
    memset(temp,0,sizeof(temp));
    int k=0;
    if(feature_num == feature_id){
        for(int i=0;i<N;i++){
            temp[tag[i]]++;
        }
        for(int i=N-1;i>=0;i--){
            if(temp[i]!=0)k++;
        }
    }
    else {
        for(int i=0;i<N;i++){
            temp[data[i][feature_id]]++;
        }
        for(int i=N-1;i>=0;i--){
            if(temp[i]!=0)k++;
        }
    }
    return k;
}

// 计算tag或别的特征的熵
double info(int data[N][M], int tag[N], int feature_id, int data_num){
    int temp[N];//different_feature_num
    double sum;
    memset(temp,0,sizeof(temp));
    // 计算tag的熵
    if(feature_num == feature_id){
        for(int i=0;i<data_num;i++){
            temp[tag[i]]++;
        }
        sum=0;
        for(int i=0;i<different_tag_num;i++){
            if(data_num == 0){
                sum += 0;
            }
            else {
                sum += info_part((double)temp[i] / (double)data_num);
            }
        }
        // printf("%lf\n",sum);
    }
    // 计算已知feature_id位置的特征时的熵
    else {
        int temp_tag[N][N];//different_feature*different_tag
        memset(temp_tag,0,sizeof(temp_tag));
        int different_feature_num = find_different_feature_num(data,tag,feature_id);
        double info;

        for(int i=0;i<data_num;i++){
            temp[data[i][feature_id]]++;
            temp_tag[data[i][feature_id]][tag[i]]++;
        }
        sum=0;
        for(int i=0;i<different_feature_num;i++){
            // printf("%d %d ",temp[i],data_num);
            double p = (double)temp[i]/(double)data_num;
            // printf("%lf ",p);
            info = 0;
            for(int j=0;j<different_tag_num;j++){
                // printf("%d %d\n",temp_tag[i][j],temp[i]);
                if (temp[i]!=0){
                    info += info_part((double)temp_tag[i][j]/(double)temp[i]);    
                }
                else {
                    info += 0;
                }
            }
            // printf("part_info:%lf\n",info);
            sum += p*info;
        }
        // printf("\tsum_info:%lf\n",sum);
    }
    return sum;
}

// 计算信息增益
double gain(int data[N][M], int tag[N], int feature_id, int data_num){
    return info(data,tag,feature_num,data_num) - info(data,tag,feature_id,data_num);
}

// 打印树的非叶子节点
void print_tree(int tag, int deep, int branch_sel, int father_feature){
    for(int i=0;i<deep;i++){
        printf("        ");
    }
    if(visual_switch){
        printf("+%s",feature_list_buffer[father_feature][branch_sel]);
        for(int i=0;i<3;i++){
            printf("-");
        }
        printf("%s\n",feature_buffer[tag]);
    }
    else if(!visual_switch){
        printf("+%2d",branch_sel);
        for(int i=0;i<5;i++){
            printf("-");
        }
        printf("%d\n",tag);
    }
    
}

// 打印树的叶子节点
void print_leaf(int tag, int deep, int branch_sel, int father_feature){
    for(int i=0;i<deep;i++){
        printf("        ");
    }
    if(visual_switch){
        printf("+%s",feature_list_buffer[father_feature][branch_sel]);
        for(int i=0;i<3;i++){
            printf("-");
        }
        printf("%s\n",tag_buffer[tag]);
    }
    else if(!visual_switch){
        printf("+%2d",branch_sel);
        for(int i=0;i<5;i++){
            printf("-");
        }
        printf("%d\n",tag);
    }
    
}

// 递归程序
// 计算总体信息熵 
//      如果为0就说明上一个特征的分支足够对其分类
//      结束递归子程序返回上一层
// 计算每个特征的信息增益 并找到信息增益最大的特征
// 按照该特征对数据集进行分裂，然后传递给下一层递归程序
int branch_process(int data[N][M], int tag[N], int data_num, int ans_tree_depth, int branch_sel, int father_feature, int used_feature[]){
    // 获得当前已经使用过的特征，直接调用会发生地址传递，所以使用这种方法
    int new_used_feature[feature_num];
    for(int i=0;i<feature_num;i++){
        new_used_feature[i] = used_feature[i];
    }
    // 计算总体信息熵
    double info_stage = info(data,tag,feature_num,data_num);
    // printf("data_num:%d info:%lf",data_num,info_stage);
    // printf("current_all_info%lf\n",info_stage);
    if(info_stage == 0) {
        ans_tree[ans_tree_line++][ans_tree_depth] = tag[0] - 2;
        // ans_tree_offset[ans_tree_depth]++;
        print_leaf(tag[0],ans_tree_depth,branch_sel,father_feature);
        // printf("\n");
        return 0;
    }
    // 寻找信息增益最大的特征
    double gain_arr[feature_num];
    int max_gain_id = 0;
    for(int i=0;i<feature_num;i++){
        gain_arr[i] = gain(data,tag,i,data_num);
        // printf("%lf ",gain_arr[i]);
        max_gain_id = gain_arr[max_gain_id] >= gain_arr[i] ? max_gain_id : i;
    }
    // printf("\nmax_gain_id:%d now_deep:%d\n",max_gain_id,ans_tree_depth);

    // 检查当前特征是否已被使用过，如果已被使用过则认为剩下的特征区分度较低，进行剪枝，默认为不好的结果
    if(new_used_feature[max_gain_id]==1){
        ans_tree[ans_tree_line++][ans_tree_depth] = 0 - 2;
        print_leaf(0,ans_tree_depth,branch_sel,father_feature);
        return 0;
    }

    // 在结果数组中，标记当前节点的位置和选择
    ans_tree[ans_tree_line++][ans_tree_depth] = max_gain_id+1;

    // 打印当前分支
    print_tree(max_gain_id, ans_tree_depth,branch_sel,father_feature);

    // 标记已经使用过的特征，避免循环调用同一个特征
    new_used_feature[max_gain_id]++;

    // 针对当前信息增益最大的特征，对数据集进行分裂，并触发下一层的递归子程序
    int different_feature_num = find_different_feature_num(source_data,tag,max_gain_id);
    for(int t=0;t<different_feature_num;t++){
        int new_data[N][M];
        int new_tag[N];
        memset(new_data,0,sizeof(new_data));
        int new_data_num=0;
        // 根据特征对数据集进行分裂
        for(int i=0;i<data_num;i++){
            if(data[i][max_gain_id]==t){
                for(int j=0;j<feature_num;j++){
                    new_data[new_data_num][j] = data[i][j];
                    new_tag[new_data_num] = tag[i];
                }
                new_data_num++;
            }  
        }    
        // 统计新数据集的样本数量
        if(new_data_num == 0){
            for(int i=0;i<feature_num;i++){
                new_data[0][i] = 1;
            }
            new_tag[0]=0;
        }
        
        // 触发下一层递归子程序
        branch_process(new_data,new_tag,new_data_num,ans_tree_depth+1,t,max_gain_id,new_used_feature);
    }
}

// 求根节点的函数 由于根节点相对于叶子节点具有一定特殊性，故单独写一个函数
int branch_launch(int data[N][M], int tag[N], int data_num, int ans_tree_depth, int branch_sel){
    int used_feature[feature_num];
    memset(used_feature,0,sizeof(used_feature));
    // 计算总体信息熵
    double info_stage = info(data,tag,feature_num,data_num);
    // printf("current_all_info%lf\n",info_stage);

    // 寻找信息增益最大的特征
    double gain_arr[feature_num];
    int max_gain_id = 0;
    for(int i=0;i<feature_num;i++){
        gain_arr[i] = gain(data,tag,i,data_num);
        // printf("%lf ",gain_arr[i]);
        max_gain_id = gain_arr[max_gain_id] >= gain_arr[i] ? max_gain_id : i;
    }
    ans_tree[ans_tree_line++][ans_tree_depth] = max_gain_id+1;
    // printf("\nmax_gain_id:%d now_deep:%d\n",max_gain_id,ans_tree_depth);

    // 标记已经使用过的特征，避免循环调用同一个特征
    used_feature[max_gain_id]++;
    // 打印根节点
    if(visual_switch){
        printf("+");
        for(int i=0;i<7;i++){
            printf("-");
        }
        printf("%s\n",feature_buffer[max_gain_id]);
    }
    else if(!visual_switch){
        printf("+");
        for(int i=0;i<7;i++){
            printf("-");
        }
        printf("%d\n",max_gain_id);
    }
    // printf("%s\n",feature_buffer[tag]);

    // 针对当前信息增益最大的特征，对数据集进行分裂，并触发下一层的递归子程序
    int different_feature_num = find_different_feature_num(source_data,tag,max_gain_id);

    for(int t=0;t<different_feature_num;t++){
        int new_data[N][M];
        int new_tag[N];
        memset(new_data,0,sizeof(new_data));
        int new_data_num=0;
        // 根据特征对数据集进行分裂
        for(int i=0;i<data_num;i++){
            if(data[i][max_gain_id]==t){
                for(int j=0;j<feature_num;j++){
                    new_data[new_data_num][j] = data[i][j];
                    new_tag[new_data_num] = tag[i];
                }
                new_data_num++;
            }  
        }    
        // 统计新数据集中样本的数量
        if(new_data_num == 0){
            for(int i=0;i<feature_num;i++){
                new_data[0][i] = 1;
            }
            new_tag[0]=0;
        }
        
        // 触发下一层递归子程序
        branch_process(new_data,new_tag,new_data_num,ans_tree_depth+1,t,max_gain_id,used_feature);
    }
}

// 预测器 通过遍历存储决策树的二维数组实现
int predict(int feature[]){
    int ans=0;
    int current_feature_sel = feature[ans_tree[0][0]-1];
    int i=0,j=0;
    int cnt = -1;
    while(1){
        i++;
        j++;
        while(1){
            // printf("%d %d %d %d\n",i,j,cnt,current_feature_sel);
            if(ans_tree[i][j]!=0){
                cnt++;
            }
            if(cnt == current_feature_sel){
                cnt = -1;
                break;
            }
            else{
                i++;
            }
            if(i==N-1){
                i=0;
                break;
            }
        }
        if(ans_tree[i][j]<0){
            ans = ans_tree[i][j]+2;
            break;
        }
        else {
            current_feature_sel = feature[ans_tree[i][j]-1];
        }
        if(i==N-1){
            ans = 0;
        }
    }
    return ans;
}

int main(){
    // 重定向输入文件 设定为训练集
    // freopen("in","r",stdin);
    // 重定向输入文件 设定为西瓜训练集
    freopen("melon","r",stdin);
    // 重定向输出文件
    freopen("out","w",stdout);

    int data[N][M];
    int data_num;
    int tag[N];
    memset(ans_tree,0,sizeof(ans_tree));
    memset(source_data,0,sizeof(source_data));
    memset(source_tag,0,sizeof(source_tag));
    memset(data,0,sizeof(data));
    scanf("%d%d",&feature_num,&data_num);

    // 获取数据集
    printf("feature number:%d\ndata num:%d\n",feature_num,data_num);
    for(int i=0;i<data_num;i++){
        for(int j=0;j<feature_num;j++){
            scanf("%d",&data[i][j]);
            source_data[i][j] = data[i][j];
        }
        scanf("%d",&tag[i]);
        source_tag[i] = tag[i];
    }
    different_tag_num = find_different_feature_num(data,tag,feature_num);

    // 生成器部分
    branch_launch(data,tag,data_num,0,0);
    printf("end\n");
    for(int i=0;i<15;i++){
        for(int j=0;j<15;j++){
            printf("%2d ",ans_tree[i][j]);
        }
        printf("\n");
    }

    // 预测器部分
    // 重定向输入文件 设定为测试集
    freopen("test","r",stdin);
    int test_arr[N];
    memset(test_arr,0,sizeof(test_arr));
    // 多组输入读取，一直读到文件尽头
    while(scanf("%d",&test_arr[0])!=EOF){
        for(int i=1;i<feature_num;i++){
            scanf("%d",&test_arr[i]);
        }
        if(visual_switch){
            // 输出特征
            for(int i=0;i<feature_num;i++){
                printf("%s ",feature_list_buffer[i][test_arr[i]]);
            }
            // printf("\n");
            int ans = predict(test_arr);
            // 输出结果
            printf("%s\n",tag_buffer[ans]);
        }
        else if(!visual_switch){
            // 输出特征
            for(int i=0;i<feature_num;i++){
                printf("%d ",test_arr[i]);
            }
            int ans = predict(test_arr);
            // 输出结果
            printf("%d\n",ans);
        }
        
    }
    return 0;
}
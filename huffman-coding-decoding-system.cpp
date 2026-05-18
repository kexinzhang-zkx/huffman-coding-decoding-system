/*
 * 项目：哈夫曼编解码系统
 * 功能：基于哈夫曼算法实现文本编码、译码、树结构调整、遍历、可视化等
 * 语言：C语言
 * 作者：kexinzhang-zkx
 * 功能清单：
 * 1. 构建哈夫曼树
 * 2. 哈夫曼编码
 * 3. 哈夫曼译码
 * 4. 三种遍历方式
 * 5. 计算带权路径长度 WPL
 * 6. 节点增删改
 * 7. 树结构可视化
 * 8. 时间/空间复杂度统计
 */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAXVALUE 10000           //定义最大权值 
#define MAXLEAF 30               //定义哈夫曼树中叶子节点个数 
#define MAXNODE MAXLEAF * 2 - 1
#define MAXBIT 50
#define MAX_INPUT_LENGTH 99  // 定义输入字符序列的最大长度（预留一个位置给'\0'）
#define MAX_NODE_COUNT 100
// 哈夫曼树节点结构体
typedef struct node {
    char letter;
    int weight;               //结点的权值 
    int parent;               //结点的双亲 
    int lchild;               //结点的左孩子 
    int rchild;               //结点的右孩子 
    int deleted;              // 新增标记，0表示未删除，1表示已删除
    int original_index;       // 新增，记录节点的原始序号
} HNodeType;

// 哈夫曼编码结构体
typedef struct {
    char letter;
    int bit[MAXBIT];
    int start;
} HCodeType;

// 用于统计字符及对应出现次数的结构体
typedef struct {
    char s;
    int num;
} Message;

// 哈夫曼树的构造算法，整合了之前HuffmanCreate和HuffmanTree的功能，直接接收输入字符串构建哈夫曼树
// 时间复杂度：该函数主要包含两层循环，时间复杂度大致为O(n^2)，其中n为不同字符的个数
int HuffmanTreeBuild(HNodeType HuffNode[], char input_str[]) {
    if (input_str[0] == '\0') {  // 增加对空字符串输入的判断
        printf("输入的字符序列为空，请重新输入非空字符序列。\n");
        return -1;
    }
    Message data[30];
    int i, count = 0;
    // 统计每个字符出现的次数
    for (i = 0; input_str[i]!= '\0' && i < MAX_INPUT_LENGTH; i++) {  // 限制输入长度
        if (!isprint(input_str[i])) {  // 判断是否为可打印字符，可根据实际需求调整合法性判断条件
            printf("输入的字符序列包含不可打印字符，请重新输入。\n");
            return -1;
        }
        int found = 0;
        for (int j = 0; j < count; j++) {
            if (data[j].s == input_str[i]) {
                data[j].num++;
                found = 1;
                break;
            }
        }
        if (!found) {
            data[count].s = input_str[i];
            data[count].num = 1;
            count++;
        }
    }
    if (i == MAX_INPUT_LENGTH) {  // 如果达到最大输入长度，提示输入过长
        printf("输入的字符序列过长，请重新输入较短的字符序列。\n");
        return -1;
    }
    // HuffNode[]初始化
    for (i = 0; i < 2 * count - 1; i++) {
        HuffNode[i].letter = '\0';
        HuffNode[i].weight = 0;
        HuffNode[i].parent = -1;
        HuffNode[i].lchild = -1;
        HuffNode[i].rchild = -1;
    }
    // 给哈夫曼树节点结构体数组赋值权值和字符
    for (i = 0; i < count; i++) {
        HuffNode[i].weight = data[i].num;
        HuffNode[i].letter = data[i].s;
    }
    // 构造哈夫曼树
    for (i = 0; i < count - 1; i++) {
        int m1 = MAXVALUE, m2 = MAXVALUE;
        int x1 = 0, x2 = 0;
        // 找到权值最小的两个未合并节点（即双亲指针为 -1 的节点）
        for (int j = 0; j < count + i; j++) {
            if (HuffNode[j].parent == -1 && HuffNode[j].weight < m1) {
                m2 = m1;
                x2 = x1;
                m1 = HuffNode[j].weight;
                x1 = j;
            } else if (HuffNode[j].parent == -1 && HuffNode[j].weight < m2) {
                m2 = HuffNode[j].weight;
                x2 = j;
            }
        }
        // 将找出的两棵子树合并为一棵子树
        // 设置第一个子树的双亲指针
        HuffNode[x1].parent = count + i;
        // 设置第二个子树的双亲指针
        HuffNode[x2].parent = count + i;
        // 计算合并后新节点的权值
        HuffNode[count + i].weight = HuffNode[x1].weight + HuffNode[x2].weight;
        // 设置新节点的左孩子指针
        HuffNode[count + i].lchild = x1;
        // 设置新节点的右孩子指针
        HuffNode[count + i].rchild = x2;
    }
    return count;
}
// 时间复杂度：该函数包含一层循环遍历叶子节点，循环内部有向上回溯到根节点的操作，时间复杂度大致为O(n log n)，n为叶子节点个数
void Encoding(HNodeType ht[], HCodeType hcd[], int n) {
    int i, j, c, p;
    HCodeType cd;
    for (i = 0; i < n; i++) {  // 遍历每个叶子节点（对应不同字符）
        cd.start = n - 1;
        c = i;
        p = ht[c].parent;
        while (p!= -1) {    // 从叶子节点向上回溯到根节点，生成编码
            if (ht[p].lchild == c)
                cd.bit[cd.start] = 0;
            else
                cd.bit[cd.start] = 1;
            cd.start--;
            c = p;
            p = ht[c].parent;
        }
        hcd[i].letter = ht[i].letter;  // 将编码对应字符赋值给HCodeType数组元素
        for (j = cd.start + 1; j < n; j++)  // 复制生成的编码到HCodeType数组
            hcd[i].bit[j] = cd.bit[j];
        hcd[i].start = cd.start;
    }
    // 输出每个字符的哈夫曼编码
    printf("输出每个字符的哈夫曼编码：\n");
    for (i = 0; i < n; i++) {
        printf(" %c:", hcd[i].letter);
        for (j = hcd[i].start + 1; j < n; j++)
            printf(" %d", hcd[i].bit[j]);
        printf("\n");
    }
}
// 时间复杂度：该函数包含一层循环遍历编码字符串，循环内部根据编码值移动到对应子树节点，时间复杂度大致为O(m)，m为编码字符串长度
void Decoding(HNodeType ht[], int n, char encoded[]) {
    int i = 0, len = strlen(encoded);
    if (len == 0) {  // 增加对输入编码字符串为空的判断
        printf("输入的编码字符串不能为空，请重新输入。\n");
        return;
    }
    if (len > MAXBIT) {  // 限制编码字符串长度，防止过长
        printf("输入的编码字符串过长，请重新输入较短的编码。\n");
        return;
    }
    int current = 2 * n - 2;  // 从根节点开始，根节点索引为2*n - 2
    printf("输出哈夫曼译码:\n");
    while (i < len) {
        if (encoded[i]!= '0' && encoded[i]!= '1') {  // 增加对输入编码合法性的判断，只能是0或1
            printf("输入的编码字符串包含非法字符，请重新输入。\n");
            return;
        }
        if (encoded[i] == '0') {
            current = ht[current].lchild;
        } else {
            current = ht[current].rchild;
        }
        if (ht[current].lchild == -1 && ht[current].rchild == -1) {  // 到达叶子节点，说明找到了一个字符
            printf("%c", ht[current].letter);
            current = 2 * n - 2;  // 回到根节点，继续译码下一段编码
        }
        i++;
    }
    printf("\n");
}
// 新添加的函数，用于前序遍历哈夫曼树并输出节点信息
void PreorderTraversal(HNodeType ht[], int index) {
    if (index == -1) {
        return;
    }
    printf("节点字符: %c, 权值: %d, 双亲节点: %d, 左孩子: %d, 右孩子: %d\n", 
    ht[index].letter, ht[index].weight, ht[index].parent, ht[index].lchild, ht[index].rchild);
    PreorderTraversal(ht, ht[index].lchild);
    PreorderTraversal(ht, ht[index].rchild);
}
// 中序遍历哈夫曼树并输出节点信息
void InorderTraversal(HNodeType ht[], int index) {
    if (index == -1) {
        return;
    }
    InorderTraversal(ht, ht[index].lchild);
    printf("节点字符: %c, 权值: %d, 双亲节点: %d, 左孩子: %d, 右孩子: %d\n", 
    ht[index].letter, ht[index].weight, ht[index].parent, ht[index].lchild, ht[index].rchild);
    InorderTraversal(ht, ht[index].rchild);
}
// 后序遍历哈夫曼树并输出节点信息
void PostorderTraversal(HNodeType ht[], int index) {
    if (index == -1) {
        return;
    }
    PostorderTraversal(ht, ht[index].lchild);
    PostorderTraversal(ht, ht[index].rchild);
    printf("节点字符: %c, 权值: %d, 双亲节点: %d, 左孩子: %d, 右孩子: %d\n", 
           ht[index].letter, ht[index].weight, ht[index].parent, ht[index].lchild, ht[index].rchild);
}
// 计算哈夫曼树的带权路径长度
int CalculateWPL(HNodeType ht[], int n) {
    int wpl = 0;
    for (int i = 0; i < n; i++) {
        int depth = 0;
        int current = i;
        while (ht[current].parent!= -1) {
            depth++;
            current = ht[current].parent;
        }
        wpl += ht[i].weight * depth;
    }
    return wpl;
}
// 估算空间复杂度的函数，计算主要数据结构占用的内存空间
void EstimateSpaceComplexity(HNodeType ht[], HCodeType hcd[], int n) {
    // 计算HNodeType数组占用空间（假设char占1字节，int占4字节，结构体对齐等按默认情况考虑）
    int huffmanTreeSize = sizeof(HNodeType) * (2 * n - 1);
    // 计算HCodeType数组占用空间
    int huffmanCodeSize = sizeof(HCodeType) * n;
    printf("哈夫曼树节点数组大致占用内存空间: %d 字节\n", huffmanTreeSize);
    printf("哈夫曼编码数组大致占用内存空间: %d 字节\n", huffmanCodeSize);
    printf("总计大致占用内存空间: %d 字节\n", huffmanTreeSize + huffmanCodeSize);
}

double CalculateAverageCodeLength(HNodeType ht[], HCodeType hcd[], int n) {
    int total_length = 0;
    for (int i = 0; i < n; i++) {
        int code_length = 0;
        for (int j = hcd[i].start + 1; j < n; j++) {
            code_length++;
        }
        total_length += code_length * ht[i].weight;
    }
    double avg_length = (double)total_length / (n > 0? n : 1);  // 防止除数为0
    return avg_length;
}
// 可视化哈夫曼树的函数，优化了节点深度计算逻辑，采用递归方式确保更准确，同时完善调试输出
// 递归计算节点深度的辅助函数，更准确地处理各种树结构情况，移到函数外部定义
int getDepth(HNodeType ht[], int index) {
    if (index == -1) {
        return 0;
    }
    int leftDepth = getDepth(ht, ht[index].lchild);
    int rightDepth = getDepth(ht, ht[index].rchild);
    return (leftDepth > rightDepth? leftDepth : rightDepth) + 1;
}
// 可视化哈夫曼树的函数，展示层级结构以及节点对应的字符
void VisualizeHuffmanTree(HNodeType ht[], int n) {
    for (int i = 0; i < 2 * n - 1; i++) {
        int depth = getDepth(ht, i);
        // 打印节点深度信息，方便调试查看是否计算正确，同时输出节点索引
        printf("节点 %d 的深度（层次）为：%d\n", i, depth);
        for (int j = 0; j < depth; j++) {
            printf("    ");  // 根据层次进行缩进，这里用4个空格表示一层缩进，可调整
        }
        // 输出节点索引、类型（叶子节点或非叶子节点）以及对应的字符信息
        if (ht[i].lchild == -1 && ht[i].rchild == -1) {
            printf("节点 %d - 叶子节点，字符：%c\n", i, ht[i].letter);
        } else {
            printf("节点 %d - 非叶子节点\n", i);
        }
    }
}
// 辅助函数：递归更新子树节点的双亲指针
void updateParentPointers(HNodeType ht[], int index, int new_parent) {
    if (index == -1) {
        return;
    }
    ht[index].parent = new_parent;
    updateParentPointers(ht, ht[index].lchild, new_parent);
    updateParentPointers(ht, ht[index].rchild, new_parent);
}
// 辅助函数：计算以某个节点为根的子树的权值总和
int calculateSubtreeWeight(HNodeType ht[], int index) {
    if (index == -1) {
        return 0;
    }
    return ht[index].weight + calculateSubtreeWeight(ht, ht[index].lchild) + calculateSubtreeWeight(ht, ht[index].rchild);
}

// 函数用于从哈夫曼树中删除指定字符对应的节点，并尽量合理地调整哈夫曼树结构
void DeleteNodeFromHuffmanTree(HNodeType ht[], int n, char target_letter) {
    int target_index = -1;
    // 先查找要删除节点的索引
    for (int i = 0; i < 2 * n - 1; i++) {
        if (ht[i].letter == target_letter) {
            target_index = i;
            break;
        }
    }
    if (target_index == -1) {
        printf("要删除的字符对应的节点不存在，请检查输入。\n");
        return;
    }
    // 标记节点为已删除
    ht[target_index].deleted = 1;
    // 判断删除的是叶子节点还是非叶子节点，分别进行不同处理
    if (ht[target_index].lchild == -1 && ht[target_index].rchild == -1) {
        // 处理删除叶子节点的情况
        int parent_index = ht[target_index].parent;
        if (parent_index!= -1) {
            // 更新双亲节点指向该叶子节点的指针（左孩子或右孩子）为 -1
            if (ht[parent_index].lchild == target_index) {
                ht[parent_index].lchild = -1;
            } else {
                ht[parent_index].rchild = -1;
            }
            // 重新计算双亲节点的权值（需要考虑其另一个子节点是否存在）
            if (ht[parent_index].lchild!= -1) {
                ht[parent_index].weight = ht[ht[parent_index].lchild].weight;
            } else if (ht[parent_index].rchild!= -1) {
                ht[parent_index].weight = ht[ht[parent_index].rchild].weight;
            } else {
                // 如果双亲节点的两个子节点都被删除了（在更复杂操作后可能出现这种情况），权值设为 0 等合适处理
                ht[parent_index].weight = 0;
            }
            // 向上回溯调整树结构，检查双亲节点是否需要重新参与合并等操作
            int current_parent = parent_index;
            while (current_parent!= -1) {
                int sibling_index = (ht[current_parent].lchild == target_index)? ht[current_parent].rchild : ht[current_parent].lchild;
                // 找到当前节点的兄弟节点
                int m1 = MAXVALUE, m2 = MAXVALUE;
                int x1 = -1, x2 = -1;
                for (int j = 0; j < 2 * n - 1; j++) {
                    if (ht[j].parent == -1 && j!= target_index && ht[j].weight < m1 && ht[j].deleted == 0) {
                        m2 = m1;
                        x2 = x1;
                        m1 = ht[j].weight;
                        x1 = j;
                    } else if (ht[j].parent == -1 && j!= target_index && ht[j].weight < m2 && ht[j].deleted == 0) {
                        m2 = ht[j].weight;
                        x2 = j;
                    }
                }
                // 重新寻找当前节点（经过删除操作后的双亲节点）与其他节点合并的最优组合（类似构建树时找最小权值子树）
                if (x1!= -1 && x2!= -1) {
                    // 如果找到合适的合并节点，进行合并操作并更新相关指针和权值
                    ht[x1].parent = current_parent;
                    ht[x2].parent = current_parent;
                    ht[current_parent].weight = ht[x1].weight + ht[x2].weight;
                    ht[current_parent].lchild = x1;
                    ht[current_parent].rchild = x2;
                    break;  // 完成一次合并后可暂时停止回溯（可根据实际情况决定是否继续向上检查调整）
                }
                current_parent = ht[current_parent].parent;
            }
        }
    } else {
        // 处理删除非叶子节点的情况（更复杂，以下是一种简化示例思路，可根据实际进一步完善）
        // 先将其左右子树的所有节点的双亲指针指向删除节点的双亲节点（假设为parent_index）
        int parent_index = ht[target_index].parent;
        if (parent_index!= -1) {
            int left_child = ht[target_index].lchild;
            int right_child = ht[target_index].rchild;
            // 递归更新左子树节点的双亲指针
            updateParentPointers(ht, left_child, parent_index);
            // 递归更新右子树节点的双亲指针
            updateParentPointers(ht, right_child, parent_index);
            // 将删除节点的左子树挂接到双亲节点（需要判断是左孩子还是右孩子挂载）
            if (ht[parent_index].lchild == target_index) {
                ht[parent_index].lchild = left_child;
            } else {
                ht[parent_index].rchild = left_child;
            }
            // 重新计算双亲节点的权值（需要综合考虑新挂载的子树以及原来可能存在的另一个子树等情况）
            int new_weight = calculateSubtreeWeight(ht, left_child) + calculateSubtreeWeight(ht, right_child);
            if (ht[parent_index].lchild == left_child && ht[parent_index].rchild!= -1) {
                new_weight += ht[ht[parent_index].rchild].weight;
            } else if (ht[parent_index].rchild == left_child && ht[parent_index].lchild!= -1) {
                new_weight += ht[ht[parent_index].lchild].weight;
            }
            ht[parent_index].weight = new_weight;
            // 同样可以考虑向上回溯调整树结构，类似叶子节点删除后的处理，此处暂省略部分重复代码逻辑展示
        }
    }
    printf("节点已成功删除，哈夫曼树结构已相应调整，不过相关编码等操作可能需要重新进行。\n");
}
// 函数用于向哈夫曼树中增加新的节点（字符及对应权值）
void AddNodeToHuffmanTree(HNodeType ht[], int *n, char new_letter, int new_weight) {
    // 先打印当前节点数量和最大叶子节点数，方便排查是否因数量上限问题导致无法添加
    printf("当前节点数量 *n = %d, MAXLEAF = %d\n", *n, MAXLEAF);
    // 检查节点数组是否已满等边界情况，这里简单处理，可根据实际完善
    if (*n >= MAXLEAF) {
        printf("哈夫曼树节点数已达上限，无法添加新节点，请检查。\n");
        return;
    }
    // 找到合适位置插入新节点（优化插入逻辑，处理特殊情况，如所有节点权值都小于新节点权值时插到末尾）
    int insert_index = 0;
    for (; insert_index < 2 * (*n) - 1; insert_index++) {
        // 打印比较信息，辅助查看查找插入位置的逻辑是否正确
        printf("正在比较节点 %d，权值为 %d，是否符合插入条件（权值 >= %d 且未删除）\n", insert_index, ht[insert_index].weight, new_weight);
        if (ht[insert_index].weight >= new_weight && ht[insert_index].deleted == 0) {
            break;
        }
    }
    // 如果循环结束都没找到合适插入位置（所有节点权值都小于新节点权值），将新节点插到末尾
    if (insert_index == 2 * (*n) - 1) {
        insert_index = 2 * (*n) - 1;
    }
    // 加强插入位置索引的合法性检查，不仅判断范围，还对可能出现的异常值进一步提示
    if (insert_index < 0 || insert_index >= 2 * (*n) - 1) {
        printf("插入位置索引 %d 错误，超出合理范围，请检查程序逻辑。\n", insert_index);
        return;
    }
    // 移动已有节点，腾出位置插入新节点（简单的数组元素后移操作示例），添加更详细调试输出查看节点移动情况
    for (int i = 2 * (*n) - 1; i > insert_index; i--) {
        printf("将节点 %d 的数据（字符: %c，权值: %d，双亲: %d，左孩子: %d，右孩子: %d，删除标记: %d，原始索引: %d）移动到节点 %d\n", 
               i - 1, ht[i - 1].letter, ht[i - 1].weight, ht[i - 1].parent, ht[i - 1].lchild, ht[i - 1].rchild, ht[i - 1].deleted, ht[i - 1].original_index, i);
        ht[i] = ht[i - 1];
    }
    // 设置新节点的信息，添加更详细调试输出确认赋值情况
    printf("即将为索引为 %d 的新节点赋值，字符为 %c，权值为 %d\n", insert_index, new_letter, new_weight);
    ht[insert_index].letter = new_letter;
    ht[insert_index].weight = new_weight;
    ht[insert_index].parent = -1;
    ht[insert_index].lchild = -1;
    ht[insert_index].rchild = -1;
    ht[insert_index].deleted = 0;
    ht[insert_index].original_index = insert_index;
    printf("已完成索引为 %d 的新节点赋值，各项信息为：字符 %c，权值 %d，双亲 %d，左孩子 %d，右孩子 %d，删除标记 %d，原始索引 %d\n",
           insert_index, ht[insert_index].letter, ht[insert_index].weight, ht[insert_index].parent, ht[insert_index].lchild,
           ht[insert_index].rchild, ht[insert_index].deleted, ht[insert_index].original_index);
    // 增加节点后，需要重新调整哈夫曼树结构，类似构建哈夫曼树的合并操作
    int count = *n + 1;
    for (int i = 0; i < count - 1; i++) {
        int m1 = MAXVALUE, m2 = MAXVALUE;
        int x1 = 0, x2 = 0;
        for (int j = 0; j < count + i; j++) {
            if (ht[j].parent == -1 && ht[j].weight < m1 && ht[j].deleted == 0) {
                m2 = m1;
                x2 = x1;
                m1 = ht[j].weight;
                x1 = j;
            }
            else if (ht[j].parent == -1 && ht[j].weight < m2 && ht[j].deleted == 0) {
                m2 = ht[j].weight;
                x2 = j;
            }
        }
        // 打印每次合并选择的子树节点信息，辅助排查合并逻辑问题，添加更多节点信息展示
        printf("第 %d 次合并，选择的两棵子树节点为：\n", i + 1);
        printf("    节点 %d：字符 %c，权值 %d，双亲 %d，左孩子 %d，右孩子 %d，删除标记 %d，原始索引 %d\n", 
               x1, ht[x1].letter, ht[x1].weight, ht[x1].parent, ht[x1].lchild, ht[x1].rchild, ht[x1].deleted, ht[x1].original_index);
        printf("    节点 %d：字符 %c，权值 %d，双亲 %d，左孩子 %d，右孩子 %d，删除标记 %d，原始索引 %d\n", 
               x2, ht[x2].letter, ht[x2].weight, ht[x2].parent, ht[x2].lchild, ht[x2].rchild, ht[x2].deleted, ht[x2].original_index);
        // 将找出的两棵子树合并为一棵子树
        ht[x1].parent = count + i;
        ht[x2].parent = count + i;
        ht[count + i].weight = ht[x1].weight + ht[x2].weight;
        ht[count + i].lchild = x1;
        ht[count + i].rchild = x2;
        // 打印合并后新节点信息，查看结构调整是否正确，添加更全面信息展示
        printf("合并后新节点 %d 的信息为：字符 %c，权值 %d，双亲 %d，左孩子 %d，右孩子 %d，删除标记 %d，原始索引 %d\n", 
               count + i, ht[count + i].letter, ht[count + i].weight, ht[count + i].parent, ht[count + i].lchild, ht[count + i].rchild, ht[count + i].deleted, ht[count + i].original_index);
    }
    *n = count;
    printf("新节点已成功添加到哈夫曼树中，哈夫曼树结构已重新调整。\n");
}
// 辅助函数：计算给定节点的双亲节点的权值（根据其孩子节点权值等情况计算）
int calculateParentWeight(HNodeType ht[], int parent_index) {
    int left_weight = (ht[parent_index].lchild!= -1)? ht[ht[parent_index].lchild].weight : 0;
    int right_weight = (ht[parent_index].rchild!= -1)? ht[ht[parent_index].rchild].weight : 0;
    return left_weight + right_weight;
}
// 辅助函数：找到哈夫曼树中当前未删除且权值最小的节点的权值（用于和修改后节点相关的比较等操作）
int findMinWeight(HNodeType ht[], int n) {
    int min_weight = __INT_MAX__;  // 使用最大整数作为初始值，确保能找到更小的权值
    for (int i = 0; i < 2 *n - 1; i++) {
        if (ht[i].deleted == 0 && ht[i].weight < min_weight) {
            min_weight = ht[i].weight;
        }
    }
    return min_weight;
}
// 函数用于修改哈夫曼树中的节点（修改字符和权值），并对哈夫曼树结构进行相应调整
void modifyNodeInHuffmanTree(HNodeType ht[], int *n, char target_char, int new_weight) {
    int target_index = -1;
    // 查找要修改的节点的索引，改进查找逻辑，添加调试输出查看查找情况
    for (int i = 0; i < 2 * *n - 1; i++) {
        if (ht[i].letter == target_char) {
            target_index = i;
            printf("找到要修改的字符对应的节点，索引为: %d\n", target_index);  // 输出找到的节点索引，方便调试
            break;
        }
    }
    if (target_index == -1) {
        printf("要修改的字符对应的节点不存在，请检查输入。\n");
        return;
    }

    // 先记录原节点的双亲、左右孩子等相关信息，方便后续调整树结构
    int original_parent = ht[target_index].parent;
    int original_lchild = ht[target_index].lchild;
    int original_rchild = ht[target_index].rchild;

    // 更新节点的权值和字符
    ht[target_index].weight = new_weight;
    ht[target_index].letter = target_char;

    // 根据节点是否为叶子节点，进行不同的树结构调整操作
    if (original_lchild == -1 && original_rchild == -1) {
        // 如果是叶子节点，需检查其双亲节点的权值等是否受影响，进行相应调整
        if (original_parent!= -1) {
            // 重新计算双亲节点的权值
            ht[original_parent].weight = calculateParentWeight(ht, original_parent);
            // 进一步检查是否需要对树结构做更多调整，比如是否引发合并等操作，这里简单示例判断权值变化是否影响整体结构
            int min_weight = findMinWeight(ht, *n);
            if (ht[original_parent].weight < min_weight) {
                // 如果双亲节点权值过小，可能需要重新合并等操作来调整树结构，此处可添加更详细逻辑
                // 例如类似构建哈夫曼树时的合并操作代码，这里暂省略详细实现
                printf("修改叶子节点后，可能影响树结构，需进一步手动调整树结构或重新构建相关编码等操作。\n");
            }
        }
    } else {
        // 如果是非叶子节点，处理更复杂，需要考虑子树的权值变化对整个树结构的影响
        // 这里简单示例重新计算以该节点为根的子树的权值，并更新其双亲节点指向它的权值引用（假设双亲节点记录了子树总权值）
        int subtree_weight = calculateSubtreeWeight(ht, target_index);
        if (original_parent!= -1) {
            if (ht[original_parent].lchild == target_index) {
                ht[original_parent].weight -= ht[target_index].weight;  // 修改为使用 weight 成员，表示原权值
                ht[original_parent].weight += subtree_weight;
            } else if (ht[original_parent].rchild == target_index) {
                ht[original_parent].weight -= ht[target_index].weight;
                ht[original_parent].weight += subtree_weight;
            }
            // 同样可以进一步判断是否需要更多树结构调整操作，此处暂省略更复杂逻辑
            printf("修改非叶子节点后，可能影响树结构，需进一步手动调整树结构或重新构建相关编码等操作。\n");
        }
    }

    printf("节点已成功修改，不过哈夫曼树结构可能需要进一步手动调整或重新构建编码等相关操作。\n");
}

int main() {
    HNodeType ht[100];  // 定义存放哈夫曼树的数组，可根据需要调整大小
    HCodeType hcd[100];  // 定义存放编码的数组，可根据需要调整大小
    int select, num;
    int n = 0;
    char input_str[MAX_INPUT_LENGTH + 1];  // 根据定义的最大长度来声明数组

    while (1) {
        printf("-----欢迎来到电文的编码和译码操作系统------\n");
        printf("1---建立哈夫曼树\n");
        printf("2---编码\n");
        printf("3---译码\n");
        printf("4---带权路径长度\n");
        printf("5---可视化哈夫曼树\n");
        printf("6---修改哈夫曼树中的节点（增删改）\n");
        printf("8---退出系统\n");
        printf("请输入您所要实现的功能：\n");
        if (scanf("%d", &select)!= 1) {
            printf("输入错误，请输入1 - 8之间的整数来选择功能。\n");
            while (getchar()!= '\n');  // 清空输入缓冲区
            continue;
        }

        switch (select) {
            case 1:
                printf("请输入一些字符（不超过%d个）:\n", MAX_INPUT_LENGTH);
                scanf("%s", input_str);
                {
                    clock_t start_build_time = clock();
                    n = HuffmanTreeBuild(ht, input_str);
                    clock_t end_build_time = clock();
                    double build_time = (double)(end_build_time - start_build_time) / CLOCKS_PER_SEC;
                    if (n >= 0) {
                        printf("哈夫曼树已成功建立！构建哈夫曼树耗时: %.6lf 秒\n");
                    }
                    printf("请选择您想要的遍历方式：\n");
                    printf("1---前序遍历建立的哈夫曼树\n");
                    printf("2---中序遍历建立的哈夫曼树\n");
                    printf("3---后序遍历建立的哈夫曼树\n");
                    scanf("%d", &num);
                    switch (num) {
                        case 1:
                            printf("哈夫曼树前序遍历结果如下：\n");
                            PreorderTraversal(ht, 2 * n - 2);
                            break;
                        case 2:
                            printf("哈夫曼树中序遍历结果如下：\n");
                            InorderTraversal(ht, 2 * n - 2);
                            break;
                        case 3:
                            printf("哈夫曼树后序遍历结果如下：\n");
                            PostorderTraversal(ht, 2 * n - 2);
                            break;
                    }
                }
                break;
            case 2:
                {
                    clock_t start_encoding_time = clock();
                    Encoding(ht, hcd, n);
                    clock_t end_encoding_time = clock();
                    double encoding_time = (double)(end_encoding_time - start_encoding_time) / CLOCKS_PER_SEC;
                    printf("编码操作耗时: %.6lf 秒\n");
                    double avg_length = CalculateAverageCodeLength(ht, hcd, n);
                    printf("哈夫曼编码的平均编码长度为: %.6lf\n", avg_length);
                }
                break;
            case 3:
                char encoded[MAXBIT + 1];
                printf("请输入要译码的电文（不超过%d位）: ", MAXBIT);
                scanf("%s", encoded);
                {
                    clock_t start_decoding_time = clock();
                    Decoding(ht, n, encoded);
                    clock_t end_decoding_time = clock();
                    double decoding_time = (double)(end_decoding_time - start_decoding_time) / CLOCKS_PER_SEC;
                    printf("译码操作耗时: %.6lf 秒\n");
                }
                break;
            case 4:
                {
                    int wpl_result = CalculateWPL(ht, n);
                    printf("wpl的长度是：%d\n");
                }
                break;
            case 5:
                if (n > 0) {
                    VisualizeHuffmanTree(ht, n);
                } else {
                    printf("请先建立哈夫曼树后再尝试可视化操作。\n");
                }
                break;
            case 6:
                {
                    int num;
                    printf("1---增加节点\n");
                    printf("2---删除节点\n");
                    printf("3---修改节点\n");
                    printf("请选择您的操作：\n");
                    scanf("%d", &num);
                    switch (num) {
                        case 1:
                            char new_char;
                            int new_weight;
                            printf("请输入要添加的字符和对应的权值：\n");
                            while (getchar()!= '\n');
                            if (scanf("%c%d", &new_char, &new_weight)!= 2) {
                                printf("输入格式错误，请重新输入字符和权值，用空格隔开。\n");
                                while (getchar()!= '\n');
                                continue;
                            }
                            AddNodeToHuffmanTree(ht, &n, new_char, new_weight);
                            printf("若需要进行编码、译码等操作，请重新执行对应功能选项。\n");
                            break;
                        case 2:
                            char target_char;
                            printf("请输入要删除的字符：\n");
                            scanf(" %c", &target_char);
                            DeleteNodeFromHuffmanTree(ht, n, target_char);
                            printf("若需要进行编码、译码等操作，请重新执行对应功能选项。\n");
                            break;
                        case 3:
                       
                         
                            printf("请输入要修改的节点对应的字符以及新的权值（用空格隔开）：\n");
                            if (scanf(" %c %d", &target_char, &new_weight)!= 2) {
                                fprintf(stderr, "输入格式错误，请按照要求输入字符和权值（用空格隔开）。\n");
                                return -1;
                            }
                            while ((getchar())!= '\n' && (getchar())!= EOF);
                            modifyNodeInHuffmanTree(ht, &n, target_char, new_weight);
                            break;
                    }
                }
                break;
            case 8:
                printf("正在退出系统，感谢使用！\n");
                exit(0);
            default:
                printf("输入的功能选项无效，请重新输入。\n");
        }
    }
    return 0;
}

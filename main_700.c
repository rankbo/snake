#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_WIDTH (10)
#define GRID_HEIGHT (20)
#define BLOCK_SIZE (4)
#define BLOCK_ROTATION (4)
#define BLOCK_ROTATION_0 (0)
#define BLOCK_ROTATION_90 (1)
#define BLOCK_ROTATION_180 (2)
#define BLOCK_ROTATION_270 (3)

int g_clearScore[] ={100, 300, 500, 800};

typedef enum {
    BLOCK_I = 0,
    BLOCK_T,
    BLOCK_O,
    BLOCK_J,
    BLOCK_L,
    BLOCK_S,
    BLOCK_Z,
    BLOCK_TYPE_COUNT,
    BLOCK_TYPE_X,
    BLOCK_TYPE_E
} E_TETRI_TYPE;

// 定义俄罗斯方块的所有形状及其旋转状态
// 格式: g_AllShapes[方块类型][旋转状态][行][列]
const char g_AllShapes[BLOCK_TYPE_COUNT][BLOCK_ROTATION][BLOCK_SIZE][BLOCK_SIZE] = {
    // I型方块 (长条)
    {{"****", "    ", "    ", "    "},   // 0°
     {"*   ", "*   ", "*   ", "*   "},   // 90°
     {"****", "    ", "    ", "    "},   // 180° (同0°)
     {"*   ", "*   ", "*   ", "*   "}},  // 270° (同90°)
    // T型方块
    {{" *  ", "*** ", "    ", "    "},   // 0°
     {"*   ", "**  ", "*   ", "    "},   // 90°
     {"*** ", " *  ", "    ", "    "},   // 180°
     {" *  ", "**  ", " *  ", "    "}},  // 270°

    // O型方块 (正方形)
    {{"**  ", "**  ", "    ", "    "},   // 0°
     {"**  ", "**  ", "    ", "    "},   // 90° (同0°)
     {"**  ", "**  ", "    ", "    "},   // 180° (同0°)
     {"**  ", "**  ", "    ", "    "}},  // 270° (同0°)
    // J型方块
    {{"*   ", "*** ", "    ", "    "},   // 0°
     {"**  ", "*   ", "*   ", "    "},   // 90°
     {"*** ", "  * ", "    ", "    "},   // 180°
     {" *  ", " *  ", "**  ", "    "}},  // 270°
    // L型方块
    {{"  * ", "*** ", "    ", "    "},   // 0°
     {"*   ", "*   ", "**  ", "    "},   // 90°
     {"*** ", "*   ", "    ", "    "},   // 180°
     {"*** ", "  * ", "    ", "    "}},  // 270°

    // S型方块
    {{" ** ", "**  ", "    ", "    "},   // 0°
     {"*   ", "**  ", " *  ", "    "},   // 90°
     {" ** ", "**  ", "    ", "    "},   // 180° (同0°)
     {"*   ", "**  ", " *  ", "    "}},  // 270° (同90°)
    // Z型方块
    {{"**  ", " ** ", "    ", "    "},    // 0°
     {" *  ", "**  ", "*   ", "    "},    // 90°
     {"**  ", " ** ", "    ", "    "},    // 180° (同0°)
     {" *  ", "**  ", "*   ", "    "}}};  // 270° (同90°)

typedef struct {
    E_TETRI_TYPE blockType; //块类型
    int rotatioin;          //旋转角度
    int lastX, lastY;       //最终X Y位置
    char blockShape[4][4];  //块数据
} BlockDef;

// 当前游戏得分
int g_Score = 0;

// 当前移动的方块的定义
BlockDef g_NewBlock;

// 第一个方块的类型
E_TETRI_TYPE g_FirstBlockType  = BLOCK_I;
// 下一个方块的类型
E_TETRI_TYPE g_NextBlockType  = BLOCK_I;

// 游戏网格的二维数组，用于存储所有方块的位置
char g_AllGrids[GRID_HEIGHT][GRID_WIDTH];

/**
 * @description: 合并网格和方块
 * @param {char} tempGrid[GRID_HEIGHT][GRID_WIDTH] 临时网格
 * @param {int} x 方块X坐标
 * @param {int} y 方块Y坐标
 * @param {char} shape[4][4] 方块形状   
 */
void mergeGirdBlock(char tempGrid[GRID_HEIGHT][GRID_WIDTH], int x, int y, char shape[4][4]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (shape[i][j] != ' ') {
                int px = x + j;
                int py = y + i;
                if (py < GRID_HEIGHT && px >= 0 && px < GRID_WIDTH) {
                    tempGrid[py][px] = '*';
                }
            }
        }
    }
}

/**
 * @description: 清除满行
 * @param {char} tempGrid[GRID_HEIGHT][GRID_WIDTH] 临时网格
 * @param {int} *linesRemoved 满行数
 */
void clearLines(char tempGrid[GRID_HEIGHT][GRID_WIDTH], int* linesRemoved) {
     for (int i = GRID_HEIGHT - 1; i >= 0; i--) {
         int isLineFull = 1;
         for (int j = 0; j < GRID_WIDTH; j++) {
             if (tempGrid[i][j] == ' ') {
                 isLineFull = 0;
                 break;
             }
         }
         if (isLineFull) {
             (*linesRemoved)++;
             for (int k = i; k > 0; k--)
                 memcpy(tempGrid[k], tempGrid[k - 1], GRID_WIDTH);
             memset(tempGrid[0], ' ', GRID_WIDTH);
             i++;
         }
     }

}


/**
 * 重设方块属性
 */
void resetBlock() {
    g_NewBlock.blockType = g_FirstBlockType;
    g_NewBlock.lastX = 0;
    g_NewBlock.lastY = 0;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        strncpy(g_NewBlock.blockShape[i], g_AllShapes[g_FirstBlockType][BLOCK_ROTATION_0][i], 4);
    }
}

/**
 * 构建当前网格状态
 */
void buildGirdState() {
    int linesCleared = 0;
    mergeGirdBlock(g_AllGrids, g_NewBlock.lastX, g_NewBlock.lastY, g_NewBlock.blockShape);
    clearLines(g_AllGrids, &linesCleared);

    if (linesCleared > 0) {
        g_Score += g_clearScore[linesCleared-1];
    }
}

// 评估函数
int evaluateGrid(char tempGrid[GRID_HEIGHT][GRID_WIDTH], int linesRemoved) {
    // 1. 空洞惩罚
    int holeCount = 0;
    for (int j = 0; j < GRID_WIDTH; j++) {
        int blockFound = 0;
        for (int i = 0; i < GRID_HEIGHT; i++) {
            if (tempGrid[i][j] != ' ') {
                blockFound = 1;
            } else if (blockFound) {
                holeCount++;
            }
        }
    }
    // 2堆叠高度惩罚（stackHeight）
    int stackHeight = 0;
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (tempGrid[i][j] != ' ') {
                stackHeight = (GRID_HEIGHT - i > stackHeight) ? GRID_HEIGHT - i : stackHeight;
                break;
            }
        }
    }
    // 3. 行变化数（rowChanges）
    int rowChanges = 0;
    for (int i = 0; i < GRID_HEIGHT; i++) {
        int prevCell = 0;
        for (int j = 0; j <= GRID_WIDTH; j++) {
            int currentCell = (j < GRID_WIDTH) ? (tempGrid[i][j] != ' ') : 0;
            if (currentCell != prevCell) {
                rowChanges++;
            }
            prevCell = currentCell;
        }
    }
    // 4. 列变化数（colChanges）
    int colChanges = 0;
    for (int j = 0; j < GRID_WIDTH; j++) {
        int prevCell = 0;
        for (int i = 0; i <= GRID_HEIGHT; i++) {
            int currentCell = (i < GRID_HEIGHT) ? (tempGrid[i][j] != ' ') : 0;
            if (currentCell != prevCell) {
                colChanges++;
            }
            prevCell = currentCell;
        }
    }
    // 5. 井惩罚（wellCount）
    int wellCount = 0;
    for (int j = 0; j < GRID_WIDTH; j++) {
        for (int i = GRID_HEIGHT - 1; i >= 0; i--) {
            if (tempGrid[i][j] != ' ') {
                break;
            }
            int leftBlocked = (j == 0) || (tempGrid[i][j - 1] != ' ');
            int rightBlocked = (j == GRID_WIDTH - 1) || (tempGrid[i][j + 1] != ' ');
            if (leftBlocked && rightBlocked) {
                int wellDepth = 1;
                for (int k = i - 1; k >= 0; k--) {
                    if (tempGrid[k][j] == ' ' && (j == 0 || tempGrid[k][j - 1] != ' ') &&
                        (j == GRID_WIDTH - 1 || tempGrid[k][j + 1] != ' ')) {
                        wellDepth++;
                    } else {
                        break;
                    }
                }
                wellCount += wellDepth * (wellDepth + 1) / 2;
                break;
            }
        }
    }

    int total = 0;
    if (linesRemoved > 0) {
        total = g_clearScore[linesRemoved-1];
    } 
    // 参数可调整
    return total - holeCount * 20 - stackHeight * 30 - rowChanges * 5 - colChanges * 5 - wellCount * 10;
}

/**
 * 判断是否可以放置方块
 * @param tempGrid 临时网格
 * @param x 方块的x坐标
 * @param y 方块的y坐标
 * @param shape 方块的形状
 * @return 是否可以放置
 *
 */
int isValidPosition(char tempGrid[GRID_HEIGHT][GRID_WIDTH], int x, int y, char shape[4][4]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (shape[i][j] != ' ') {
                int px = x + j;
                int py = y + i;
                if (px < 0 || px >= GRID_WIDTH || py >= GRID_HEIGHT) {
                    return 0;
                }
                if (tempGrid[py][px] != ' ') {
                    return 0;
                }
            }
        }
    }
    return 1;
}

/**
 * 放置方块
 * @param tempGrid 临时网格
 * @param x 方块的x坐标
 * @param y 方块的y坐标
 * @param shape 方块的形状
 */
int dropToBottom(char tempGrid[GRID_HEIGHT][GRID_WIDTH], int x, int y, char shape[4][4]) {
    int py = y;
    while (isValidPosition(tempGrid, x, py, shape))
        py++;
    return py - 1;
}

// 生成所有旋转状态
void getAllRotations(E_TETRI_TYPE type, char out[4][BLOCK_SIZE][BLOCK_SIZE]) {
    for (int r = 0; r < BLOCK_ROTATION; r++) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                out[r][i][j] = g_AllShapes[type][r][i][j];
            }
        }
    }
}

// 两阶段搜索
void twoPhaseSearch() {
    int bestScore = INT_MIN;

    resetBlock();
    // 优化：静态复用棋盘，减少栈上多次分配
    // static char grid1[GRID_HEIGHT][GRID_WIDTH];
    //  static char grid2[GRID_HEIGHT][GRID_WIDTH];

    // todo 指针指向
    char curRot[BLOCK_ROTATION][BLOCK_SIZE][BLOCK_SIZE];
    getAllRotations(g_NewBlock.blockType, curRot);

    int curRotCount = BLOCK_ROTATION, nextRotCount = BLOCK_ROTATION;
    // O型方块旋转后形状不变
    if (g_NewBlock.blockType == BLOCK_O)
        curRotCount = 1;
    if (g_NextBlockType == BLOCK_O)
        nextRotCount = 1;

    for (int rot1 = 0; rot1 < curRotCount; rot1++) {
        for (int x1 = 0; x1 < GRID_WIDTH ; x1++) {
            int gLinesRemoved = 0;
            // 复制棋盘
            char grid1[GRID_HEIGHT][GRID_WIDTH];
            for (int i = 0; i < GRID_HEIGHT; i++)
                memcpy(grid1[i], g_AllGrids[i], GRID_WIDTH);

            // 下落
            int y1 = dropToBottom(grid1, x1, 0, curRot[rot1]);
            if (!isValidPosition(grid1, x1, y1, curRot[rot1]))
                continue;
            mergeGirdBlock(grid1, x1, y1, curRot[rot1]);
            clearLines(grid1, &gLinesRemoved);

            int bestNext = INT_MIN;

            //// 第二阶段，枚举下一个方块所有可能
            //if (g_NextBlockType < BLOCK_TYPE_COUNT) {
            //    // todo 指针指向
            //    char nextRot[BLOCK_ROTATION][BLOCK_SIZE][BLOCK_SIZE];
            //    getAllRotations(g_NextBlockType, nextRot);
            //    for (int rot2 = 0; rot2 < nextRotCount; rot2++) {
            //        for (int x2 = 0; x2 < GRID_WIDTH; x2++) {
            //            int secondLineRmoved = 0;
            //            int y2 = dropToBottom(grid1, x2, 0, nextRot[rot2]);
            //            if (!isValidPosition(grid1, x2, y2, nextRot[rot2]))
            //                continue;
            //            char grid2[GRID_HEIGHT][GRID_WIDTH];
            //            for (int i = 0; i < GRID_HEIGHT; i++)
            //                memcpy(grid2[i], grid1[i], GRID_WIDTH);
            //            mergeGirdBlock(grid2, x2, y2, nextRot[rot2]);
            //            clearLines(grid2, &secondLineRmoved);
            //            int score = evaluateGrid(grid2, (gLinesRemoved + secondLineRmoved));
            //            if (score > bestNext)
            //                bestNext = score;
            //        }
            //    }
            //} else {
                bestNext = evaluateGrid(grid1, gLinesRemoved);
           // }

            if (bestNext > bestScore) {
                bestScore = bestNext;
                g_NewBlock.rotatioin = rot1;
                g_NewBlock.lastX = x1;
                g_NewBlock.lastY = y1;
            }
        }
    }

    if (g_NewBlock.rotatioin != 0) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            for (int j = 0; j < BLOCK_SIZE; j++) {
                g_NewBlock.blockShape[i][j] = g_AllShapes[g_NewBlock.blockType][g_NewBlock.rotatioin][i][j];
            }
        }
    }

    buildGirdState();
}

// 其它输入输出函数与原有一致
E_TETRI_TYPE convertCharToShape(char c) {
    switch (c) {
        case 'I':
            return BLOCK_I;
        case 'T':
            return BLOCK_T;
        case 'O':
            return BLOCK_O;
        case 'J':
            return BLOCK_J;
        case 'L':
            return BLOCK_L;
        case 'S':
            return BLOCK_S;
        case 'Z':
            return BLOCK_Z;
        case 'X':
            return BLOCK_TYPE_X;
        case 'E':
            exit(0);
        default:
            exit(0);
    }
}


void readBlock() {
    char block1;
    char line[100]; // 缓冲区

    fgets(line, 100, stdin); // 读取整行
    int len = strlen(line);
    printf("长度：%d\n", len);

    printf("你输入的内容是：%s \n", line);


    //if(len == 1 || len == 2) {
    //    if (len == 1) {
    //        block1 = line[0];
    //    } else {
    //        block1 = line[0];
    //        char block2 = line[1];
    //        g_FirstBlockType = convertCharToShape(block1);
    //        g_NextBlockType = convertCharToShape(block2);
    //    }
    //} else {
    //  //.  exit(0);
    //}

    // if (firstRead) {
    //     char block2;
    //     firstRead = 0;
    //     scanf("%c%c", &block1, &block2);
    //     g_FirstBlockType = convertCharToShape(block1);
    //     g_NextBlockType = convertCharToShape(block2);
    // } else {
    //     scanf("%c", &block1);
    //     g_FirstBlockType = g_NextBlockType;
    //     g_NextBlockType = convertCharToShape(block1);
    // }
    // while (getchar() != '\n')
    //     ;
}

void writeResult() {
    printf("%d %d\n", g_NewBlock.rotatioin, g_NewBlock.lastX);
    printf("%d\n", g_Score);
    fflush(stdout);
}

void show() {
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (g_AllGrids[i][j] != ' ') {
                printf("%c", g_AllGrids[i][j]);
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }
}

// 主循环
int main() {

    memset(g_AllGrids, ' ', GRID_HEIGHT * GRID_WIDTH);
    while (1) {
        readBlock();
        //twoPhaseSearch();
        ////show();
        //writeResult();
        //if (g_NextBlockType == BLOCK_TYPE_X)
        //    break;
    }

    return 0;
}
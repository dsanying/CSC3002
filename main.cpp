// 版本号: 1.0
// 作者: dsanying
// 版本简介: 初始框架

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>

using namespace std;

// 全局变量
const int EASY = 8;     // 简单难度
const int MEDIUM = 16;  // 中等难度
const int HARD = 24;    // 困难难度

int rows, cols, mines;  // 行数、列数、地雷数
vector<vector<char>> board;  // 游戏棋盘
vector<vector<bool>> revealed;  // 记录哪些格子已经被揭开
vector<vector<bool>> flagged;  // 记录哪些格子被标记为地雷

// ANSI 转义码
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string BLUE = "\033[34m";

// 初始化游戏
void initializeGame(int difficulty) {
	if (difficulty == EASY) {
		rows = 8;
		cols = 8;
		mines = 10;
	} else if (difficulty == MEDIUM) {
		rows = 16;
		cols = 16;
		mines = 40;
	} else if (difficulty == HARD) {
		rows = 24;
		cols = 24;
		mines = 99;
	}
	
	// 初始化棋盘和状态
	board.assign(rows, vector<char>(cols, '0'));
	revealed.assign(rows, vector<bool>(cols, false));
	flagged.assign(rows, vector<bool>(cols, false));
}

// 选择游戏难度界面
void startOptionsInterface() {
	int choice;
	cout << "选择游戏难度:" << endl;
	cout << "1. 简单" << endl;
	cout << "2. 中等" << endl;
	cout << "3. 困难" << endl;
	cin >> choice;
	
	switch (choice) {
	case 1:
		initializeGame(EASY);
		break;
	case 2:
		initializeGame(MEDIUM);
		break;
	case 3:
		initializeGame(HARD);
		break;
	default:
		cout << "无效选择。退出。" << endl;
		exit(0);
	}
}

// 打印棋盘
void printBoard() {
	// 打印列坐标
	cout << "  ";
	for (int j = 0; j < cols; ++j) {
		cout << BLUE << j << RESET << " ";
	}
	cout << endl;
	
	// 打印行坐标和棋盘内容
	for (int i = 0; i < rows; ++i) {
		// 打印行坐标
		cout << GREEN << i << RESET << " ";
		for (int j = 0; j < cols; ++j) {
			if (revealed[i][j]) {
				cout << board[i][j] << " ";  // 已揭开的格子
			} else if (flagged[i][j]) {
				cout << "F ";  // 被标记为地雷的格子
			} else {
				cout << ". ";  // 未揭开的格子
			}
		}
		cout << endl;
	}
}

// 放置地雷
void placeMines() {
	srand(time(0));
	int placedMines = 0;
	while (placedMines < mines) {
		int x = rand() % rows;
		int y = rand() % cols;
		if (board[x][y] != 'M') {
			board[x][y] = 'M';  // 放置地雷
			placedMines++;
		}
	}
}

// 计算每个格子周围的地雷数
void calculateNumbers() {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			if (board[i][j] == 'M') continue;
			int count = 0;
			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					int nx = i + dx;
					int ny = j + dy;
					if (nx >= 0 && nx < rows && ny >= 0 && ny < cols && board[nx][ny] == 'M') {
						count++;
					}
				}
			}
			board[i][j] = '0' + count;  // 设置周围地雷数
		}
	}
}

// 递归揭开格子
void reveal(int x, int y) {
	if (x < 0 || x >= rows || y < 0 || y >= cols || revealed[x][y]) return;
	revealed[x][y] = true;
	if (board[x][y] == '0') {
		for (int dx = -1; dx <= 1; ++dx) {
			for (int dy = -1; dy <= 1; ++dy) {
				reveal(x + dx, y + dy);
			}
		}
	}
}

// 左键点击
void leftClick(int x, int y) {
	if (board[x][y] == 'M') {
		cout << "游戏结束！你踩到了地雷。" << endl;
		cout << "踩到的地雷位置: (" << x << ", " << y << ")" << endl;
		cout << "整个棋盘揭开的样子:" << endl;
		// 揭开所有格子
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				revealed[i][j] = true;
			}
		}
		printBoard();
		// 询问是否重新开始或退出
		char choice;
		cout << "游戏结束。输入 'q' 退出，或输入 's' 重新开始: ";
		cin >> choice;
		if (choice == 'q') {
			cout << "退出游戏。" << endl;
			exit(0);
		} else if (choice == 's') {
			cout << "重新开始游戏..." << endl;
			startOptionsInterface();  // 选择游戏难度
			placeMines();  // 放置地雷
			calculateNumbers();  // 计算每个格子周围的地雷数
		} else {
			cout << "无效选择。退出。" << endl;
			exit(0);
		}
	} else {
		reveal(x, y);
	}
}

// 右键点击
void rightClick(int x, int y) {
	flagged[x][y] = !flagged[x][y];  // 切换标记状态
}

// 检查游戏是否胜利
bool checkWin() {
	int correctRevealed = 0;
	int correctFlagged = 0;
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			if (revealed[i][j] && board[i][j] != 'M') {
				correctRevealed++;
			}
			if (flagged[i][j] && board[i][j] == 'M') {
				correctFlagged++;
			}
		}
	}
	if (correctRevealed + mines == rows * cols) {
		cout << "游戏胜利！" << endl;
		cout << "正确揭开的格子数: " << correctRevealed << endl;
		cout << "正确标记的地雷数: " << correctFlagged << endl;
		cout << "整个棋盘揭开的样子:" << endl;
		// 揭开所有格子
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				revealed[i][j] = true;
			}
		}
		printBoard();
		return true;
	}
	return false;
}

int main() {
	startOptionsInterface();  // 选择游戏难度
	placeMines();  // 放置地雷
	calculateNumbers();  // 计算每个格子周围的地雷数
	
	while (true) {
		printBoard();  // 打印棋盘
		int x, y;
		char action;
		cout << "输入坐标 (x y) 和操作 (l 为左键点击, r 为右键点击): ";
		cin >> x >> y >> action;
		
		// 检查输入是否有效
		if (cin.fail()) {
			cin.clear();  // 清除错误状态
			cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 忽略无效输入
			cout << "无效输入，请重新输入。" << endl;
			continue;
		}
		
		if (action == 'l') {
			leftClick(x, y);  // 左键点击
		} else if (action == 'r') {
			rightClick(x, y);  // 右键点击
		} else {
			cout << "无效操作，请重新输入。" << endl;
		}
		
		if (checkWin()) {
			// 询问是否重新开始或退出
			char choice;
			cout << "游戏结束。输入 'q' 退出，或输入 's' 重新开始: ";
			cin >> choice;
			if (choice == 'q') {
				cout << "退出游戏。" << endl;
				exit(0);
			} else if (choice == 's') {
				cout << "重新开始游戏..." << endl;
				startOptionsInterface();  // 选择游戏难度
				placeMines();  // 放置地雷
				calculateNumbers();  // 计算每个格子周围的地雷数
			} else {
				cout << "无效选择。退出。" << endl;
				exit(0);
			}
		}
	}
	
	return 0;
}

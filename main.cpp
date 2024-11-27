/*
* Copyright (c) 2024
* Version: 2.0
* Author: dsanying
*
* Update Log:
* - 添加难度选择
* - 添加用户系统
* - 添加历史战绩系统
* - 优化代码逻辑
*/


#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <unistd.h>  // 用于清除屏幕
#include <chrono>    // 用于计时
#include <string>    // 用于处理用户名
#include <fstream>   // 用于文件操作
#include <iomanip>   // 用于格式化时间

using namespace std;

// 全局变量
const int EASY = 8; // 简单难度
const int MEDIUM = 16; // 中等难度
const int HARD = 24; // 困难难度

int rows, cols, mines; // 行数、列数、地雷数
vector<vector<char>> board; // 游戏棋盘
vector<vector<bool>> revealed; // 记录哪些格子已经被揭开
vector<vector<bool>> flagged; // 记录哪些格子被标记为地雷
string username; // 用户名
string gameMode; // 游戏模式
string gameDifficulty; // 游戏难度

// ANSI 转义码
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string BRIGHT_BLUE = "\033[94m"; // 亮蓝色
const string YELLOW = "\033[33m"; // 黄色，用于高亮显示

// 函数声明
void initializeGame();
void startOptionsInterface();
void printBoard();
void placeMines();
void calculateNumbers();
void reveal ( int x, int y );
void leftClick ( int x, int y );
void rightClick ( int x, int y );
bool checkWin();
void clearScreen();
void login();
void logout();
void showMenu();
void saveGameRecord ( int rows, int cols, int mines, int duration, bool win );
void showHistory();
void handleInvalidInput();

// 计时相关变量
chrono::steady_clock::time_point startTime;

// 点击事件计数器
int leftClickCount = 0;
int rightClickCount = 0;

// 初始化游戏
void initializeGame() {
	// 初始化棋盘和状态
	board.assign ( rows, vector<char> ( cols, '0' ) );
	revealed.assign ( rows, vector<bool> ( cols, false ) );
	flagged.assign ( rows, vector<bool> ( cols, false ) );
	
	// 重置点击事件计数器
	leftClickCount = 0;
	rightClickCount = 0;
}

// 选择游戏模式界面
void startOptionsInterface() {
	int choice;
	while (true) {
		clearScreen(); // 清除屏幕
		cout << "选择游戏模式:" << endl;
		cout << "1. 经典模式" << endl;
		cout << "2. 残局模式" << endl;
		cin >> choice;
		
		if (cin.fail()) {
			handleInvalidInput();
			continue;
		}
		
		switch (choice) {
		case 1:
			gameMode = "经典模式";
			break;
			
		case 2:
			gameMode = "残局模式";
			break;
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
			continue;
		}
		
		break;
	}
	
	// 选择游戏难度
	while (true) {
		clearScreen(); // 清除屏幕
		cout << "选择游戏难度:" << endl;
		cout << "1. 简单 (8x8)" << endl;
		cout << "2. 中等 (16x16)" << endl;
		cout << "3. 困难 (24x24)" << endl;
		cout << "4. 自定义" << endl;
		cin >> choice;
		
		if (cin.fail()) {
			handleInvalidInput();
			continue;
		}
		
		switch (choice) {
		case 1:
			rows = EASY;
			cols = EASY;
			mines = 10;
			gameDifficulty = "简单";
			break;
			
		case 2:
			rows = MEDIUM;
			cols = MEDIUM;
			mines = 40;
			gameDifficulty = "中等";
			break;
			
		case 3:
			rows = HARD;
			cols = HARD;
			mines = 99;
			gameDifficulty = "困难";
			break;
			
		case 4:
			gameDifficulty = "自定义";
			while (true) {
				clearScreen(); // 清除屏幕
				cout << "请输入棋盘行数: ";
				cin >> rows;
				if (cin.fail() || rows <= 0) {
					handleInvalidInput();
					continue;
				}
				cout << "请输入棋盘列数: ";
				cin >> cols;
				if (cin.fail() || cols <= 0) {
					handleInvalidInput();
					continue;
				}
				cout << "请输入地雷数量: ";
				cin >> mines;
				if (cin.fail() || mines <= 0 || mines >= rows * cols) {
					handleInvalidInput();
					continue;
				}
				break;
			}
			break;
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
			continue;
		}
		
		break;
	}
	
	initializeGame(); // 初始化游戏
	
	// 初始化残局模式，揭开部分格子
	if (gameMode == "残局模式") {
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				if (rand() % 2 == 0) {
					revealed[i][j] = true;
				}
			}
		}
	}
}

// 打印棋盘
void printBoard() {
	// 计算最大列数的宽度
	int maxColWidth = to_string(cols - 1).length();
	// 计算最大行数的宽度
	int maxRowWidth = to_string(rows - 1).length();
	// 计算每个格子的宽度
	int cellWidth = max(maxColWidth, 2); // 至少为2，以容纳数字和点
	
	// 打印列坐标
	cout << "  ";
	for (int j = 0; j < cols; ++j) {
		cout << BRIGHT_BLUE << setw(cellWidth) << j << RESET << " ";
	}
	cout << endl;
	
	// 打印行坐标和棋盘内容
	for (int i = 0; i < rows; ++i) {
		// 打印行坐标
		cout << GREEN << setw(maxRowWidth) << i << RESET << " ";
		
		for (int j = 0; j < cols; ++j) {
			if (revealed[i][j]) {
				if (board[i][j] == 'M') {
					cout << RED << setw(cellWidth) << "M" << RESET << " "; // 用红色标记地雷
				} else {
					cout << setw(cellWidth) << board[i][j] << " "; // 已揭开的格子
				}
			} else if (flagged[i][j]) {
				cout << setw(cellWidth) << "F" << " "; // 被标记为地雷的格子
			} else {
				cout << setw(cellWidth) << "." << " "; // 未揭开的格子
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
			board[x][y] = 'M'; // 放置地雷
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
			
			board[i][j] = '0' + count; // 设置周围地雷数
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
	if (x < 0 || x >= rows || y < 0 || y >= cols) {
		clearScreen();
		cout << "无效坐标，请重新输入。" << endl;
		return;
	}
	
	if (board[x][y] == 'M') {
		clearScreen(); // 清除屏幕
		cout << YELLOW << "游戏结束！你踩到了地雷。" << RESET << endl;
		cout << "踩到的地雷位置: (" << x << ", " << y << ")" << endl;
		cout << "整个棋盘揭开的样子:" << endl;
		
		// 揭开所有格子
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				revealed[i][j] = true;
			}
		}
		
		printBoard();
		// 计算并显示游戏时间
		auto endTime = chrono::steady_clock::now();
		auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
		cout << "游戏时间: " << duration << " 秒" << endl;
		// 显示点击事件次数
		cout << "有效左键点击次数: " << leftClickCount << endl;
		cout << "有效右键点击次数: " << rightClickCount << endl;
		// 保存游戏记录
		saveGameRecord(rows, cols, mines, duration, false);
		// 询问是否重新开始或返回菜单
		char choice;
		
		while (true) {
			cout << "游戏结束。输入 'm' 返回菜单，或输入 's' 重新开始: ";
			cin >> choice;
			
			if (choice == 'm') {
				showMenu();
				break;
			} else if (choice == 's') {
				clearScreen(); // 清除屏幕
				startOptionsInterface(); // 选择游戏模式
				placeMines(); // 放置地雷
				calculateNumbers(); // 计算每个格子周围的地雷数
				startTime = chrono::steady_clock::now(); // 重置计时器
				break;
			} else {
				clearScreen();
				cout << "无效选择。请重新输入。" << endl;
			}
		}
	} else {
		reveal(x, y);
		leftClickCount++; // 增加左键点击计数
	}
}

// 右键点击
void rightClick(int x, int y) {
	if (x < 0 || x >= rows || y < 0 || y >= cols) {
		clearScreen();
		cout << "无效坐标，请重新输入。" << endl;
		return;
	}
	
	flagged[x][y] = !flagged[x][y]; // 切换标记状态
	rightClickCount++; // 增加右键点击计数
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
		clearScreen(); // 清除屏幕
		cout << YELLOW << "游戏胜利！" << RESET << endl;
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
		// 计算并显示游戏时间
		auto endTime = chrono::steady_clock::now();
		auto duration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
		cout << "游戏时间: " << duration << " 秒" << endl;
		// 显示点击事件次数
		cout << "有效左键点击次数: " << leftClickCount << endl;
		cout << "有效右键点击次数: " << rightClickCount << endl;
		// 保存游戏记录
		saveGameRecord(rows, cols, mines, duration, true);
		return true;
	}
	
	return false;
}

// 清除屏幕
void clearScreen() {
	cout << "\033[2J\033[1;1H"; // 使用ANSI转义码清除屏幕
}

// 登录
void login() {
	clearScreen(); // 清除屏幕
	cout << "请输入用户名: ";
	cin >> username;
	cout << "欢迎，" << username << "！" << endl;
}

// 退出登录
void logout() {
	clearScreen(); // 清除屏幕
	cout << "退出登录。" << endl;
	username = "";
	login(); // 重新登录
	showMenu(); // 重新显示菜单
}

// 显示菜单
void showMenu() {
	int choice;
	while (true) {
		clearScreen(); // 清除屏幕
		cout << "菜单:" << endl;
		cout << "1. 开始游戏" << endl;
		cout << "2. 历史战绩" << endl;
		cout << "3. 退出登录" << endl;
		cin >> choice;
		
		if (cin.fail()) {
			handleInvalidInput();
			continue;
		}
		
		switch (choice) {
		case 1:
			clearScreen(); // 清除屏幕
			startOptionsInterface(); // 选择游戏模式
			placeMines(); // 放置地雷
			calculateNumbers(); // 计算每个格子周围的地雷数
			startTime = chrono::steady_clock::now(); // 重置计时器
			return;
			
		case 2:
			clearScreen(); // 清除屏幕
			showHistory(); // 显示历史战绩
			return;
			
		case 3:
			logout(); // 退出登录
			return;
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
		}
	}
}

// 保存游戏记录
void saveGameRecord(int rows, int cols, int mines, int duration, bool win) {
	ofstream file(username + "_history.txt", ios::app);
	
	if (file.is_open()) {
		auto now = chrono::system_clock::now();
		auto now_time_t = chrono::system_clock::to_time_t(now);
		file << put_time(localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << " " << gameMode << " " << gameDifficulty << " " << rows << "x" << cols << " " << mines << " " << duration << " " << (win ? "成功" : "失败") << endl;
		file.close();
	} else {
		cout << "无法保存游戏记录。" << endl;
	}
}

// 显示历史战绩
void showHistory() {
	ifstream file(username + "_history.txt");
	
	if (file.is_open()) {
		clearScreen(); // 清除屏幕
		cout << "历史战绩:" << endl;
		string line;
		
		while (getline(file, line)) {
			istringstream iss(line);
			string date, time, mode, difficulty, size, mines, duration, result;
			
			if (iss >> date >> time >> mode >> difficulty >> size >> mines >> duration >> result) {
				cout << "时间: " << date << " " << time << ", 模式: " << mode << ", 难度: " << difficulty << ", 棋盘大小: " << size << ", 地雷数量: " << mines << ", 游戏时间: " << duration << " 秒, 结果: " << result << endl;
			}
		}
		
		file.close();
	} else {
		clearScreen(); // 清除屏幕
		cout << "没有历史战绩。" << endl;
	}
	
	char choice;
	
	while (true) {
		cout << "输入 'm' 返回菜单: ";
		cin >> choice;
		
		if (choice == 'm') {
			clearScreen(); // 清除屏幕
			showMenu();
			break;
		} else {
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
		}
	}
}

// 处理无效输入
void handleInvalidInput() {
	clearScreen(); // 清除屏幕
	cin.clear(); // 清除错误状态
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略无效输入
	cout << "无效输入，请重新输入。" << endl;
}

int main() {
	login(); // 登录
	showMenu(); // 显示菜单
	
	while (true) {
		clearScreen(); // 清除屏幕
		printBoard(); // 打印棋盘
		int x, y;
		char action;
		cout << "输入坐标 (x y) 和操作 (l 为左键点击, r 为右键点击): ";
		cin >> x >> y >> action;
		
		// 检查输入是否有效
		if (cin.fail()) {
			handleInvalidInput();
			continue;
		}
		
		if (action == 'l') {
			leftClick(x, y); // 左键点击
		} else if (action == 'r') {
			rightClick(x, y); // 右键点击
		} else {
			clearScreen();
			cout << "无效操作，请重新输入。" << endl;
		}
		
		if (checkWin()) {
			// 询问是否重新开始或返回菜单
			char choice;
			
			while (true) {
				cout << "游戏结束。输入 'm' 返回菜单，或输入 's' 重新开始: ";
				cin >> choice;
				
				if (choice == 'm') {
					showMenu();
					break;
				} else if (choice == 's') {
					clearScreen(); // 清除屏幕
					startOptionsInterface(); // 选择游戏模式
					placeMines(); // 放置地雷
					calculateNumbers(); // 计算每个格子周围的地雷数
					startTime = chrono::steady_clock::now(); // 重置计时器
					break;
				} else {
					clearScreen();
					cout << "无效选择。请重新输入。" << endl;
				}
			}
		}
	}
	
	return 0;
}

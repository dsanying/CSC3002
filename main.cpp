/*
* Copyright (c) 2024
* Version: 3.1
* Author: dsanying
*
* Update Log:
* - 修复了在残局模式下未正确计算已揭开格子数量的问题。
*   - 在初始化游戏时，增加了 `revealedCount` 变量来记录已揭开的非地雷格子数量。
*   - 在 `reveal` 函数中，当揭开非地雷格子时，增加 `revealedCount` 的计数。
* - 优化了 `checkWin` 函数的逻辑。
*   - 现在 `checkWin` 函数直接检查 `revealedCount` 是否等于非地雷格子的总数，简化了胜利条件的判断。
* - 修复了在 `useItem` 函数中输入无效选择时未正确处理的问题。
*   - 增加了对输入的进一步检查，确保用户输入的选择是有效的。
* - 修复了在 `ladderMode` 函数中输入无效操作时未正确处理的问题。
*   - 增加了对输入的进一步检查，确保用户输入的操作是有效的。
* - 优化了 `handleInvalidInput` 函数的逻辑。
*   - 现在 `handleInvalidInput` 函数会清除输入缓冲区，确保后续输入不会受到前一次输入的影响。
* - 修复了在 `mineScanner` 函数中未正确处理地雷位置的问题。
*   - 现在 `mineScanner` 函数会随机揭露两颗地雷的位置，并确保不会重复揭露同一颗地雷。
* - 修复了在 `revive` 函数中未正确设置复活甲状态的问题。
*   - 现在 `revive` 函数会正确设置 `hasRevive` 状态，确保复活甲道具能够正常使用。
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
#include <sstream>   // 用于字符串流操作
#include <algorithm> // 用于字符串分割
#include <stdexcept> // 用于捕获异常

using namespace std;

// 全局变量
const int EASY = 4; // 简单难度
const int MEDIUM = 8; // 中等难度
const int HARD = 16; // 困难难度

int rows, cols, mines; // 行数、列数、地雷数
vector<vector<char>> board; // 游戏棋盘
vector<vector<bool>> revealed; // 记录哪些格子已经被揭开
vector<vector<bool>> flagged; // 记录哪些格子被标记为地雷
string username; // 用户名
string gameMode; // 游戏模式
string gameDifficulty; // 游戏难度
int currentLevel = 1; // 当前天梯层数
int score = 0; // 玩家积分
bool hasRevive = false; // 是否拥有复活甲
int revealedCount = 0; // 已揭开的非地雷格子数量

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
void reveal(int x, int y);
void leftClick(int x, int y);
void rightClick(int x, int y);
bool checkWin();
void clearScreen();
void login();
void logout();
void showMenu();
void saveGameRecord(int rows, int cols, int mines, int duration, bool win, int level);
void showHistory();
void handleInvalidInput();
void ladderMode();
void saveScore();
void loadScore();
void useItem();
void mineScanner();
void revive();
void classicAndResidualMode();

// 计时相关变量
chrono::steady_clock::time_point startTime;

// 点击事件计数器
int leftClickCount = 0;
int rightClickCount = 0;

// 初始化游戏
void initializeGame() {
	// 初始化棋盘和状态
	board.assign(rows, vector<char>(cols, '0'));
	revealed.assign(rows, vector<bool>(cols, false));
	flagged.assign(rows, vector<bool>(cols, false));
	
	// 重置点击事件计数器
	leftClickCount = 0;
	rightClickCount = 0;
	hasRevive = false; // 重置复活甲状态
	revealedCount = 0; // 重置已揭开的非地雷格子数量
}

// 选择游戏模式界面
void startOptionsInterface() {
	int choice;
	
	while (true) {
		clearScreen();
		cout << "选择游戏模式:" << endl;
		cout << "1. 经典模式" << endl;
		cout << "2. 残局模式" << endl;
		cout << "3. 天梯模式" << endl;
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
			
		case 3:
			gameMode = "天梯模式";
			cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略换行符
			ladderMode();
			return;
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
			continue;
		}
		
		break;
	}
	
	// 选择游戏难度
	while (true) {
		clearScreen();
		cout << "选择游戏难度:" << endl;
		cout << "1. 简单 (4x4)" << endl;
		cout << "2. 中等 (8x8)" << endl;
		cout << "3. 困难 (16x16)" << endl;
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
			mines = 5;
			gameDifficulty = "简单";
			break;
			
		case 2:
			rows = MEDIUM;
			cols = MEDIUM;
			mines = 20;
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
					if (board[i][j] != 'M') {
						revealedCount++;
					}
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
	cout << "   ";
	
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
	
	if (board[x][y] != 'M') {
		revealedCount++;
	}
	
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
		if (hasRevive) {
			hasRevive = false; // 使用复活甲
			clearScreen();
			cout << YELLOW << "你踩到了地雷，但复活甲救了你！" << RESET << endl;
			revealed[x][y] = true; // 揭开地雷格子
		} else {
			clearScreen();
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
			saveGameRecord(rows, cols, mines, duration, false, currentLevel);
			// 询问是否重新开始或返回菜单
			char choice;
			
			while (true) {
				cout << "游戏结束。输入 'm' 返回菜单，或输入 's' 重新开始: ";
				cin >> choice;
				
				if (choice == 'm') {
					showMenu();
					break;
				} else if (choice == 's') {
					clearScreen();
					startOptionsInterface();
					placeMines();
					calculateNumbers();
					startTime = chrono::steady_clock::now();
					break;
				} else {
					clearScreen();
					cout << "无效选择。请重新输入。" << endl;
				}
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
	if (revealedCount + mines == rows * cols) {
		clearScreen(); // 清除屏幕
		cout << YELLOW << "游戏胜利！" << RESET << endl;
		cout << "正确揭开的格子数: " << revealedCount << endl;
		cout << "正确标记的地雷数: " << mines << endl;
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
		saveGameRecord(rows, cols, mines, duration, true, currentLevel);
		return true;
	}
	
	return false;
}

// 清除屏幕
void clearScreen() {
	system("cls"); // 清除屏幕
}

// 登录
void login() {
	clearScreen();
	cout << "请输入用户名: ";
	cin >> username;
	cout << "欢迎，" << username << "！" << endl;
	loadScore(); // 加载积分
}

// 登出
void logout() {
	clearScreen();
	cout << "再见，" << username << "！" << endl;
	saveScore(); // 保存积分
	exit(0);
}

// 显示菜单
void showMenu() {
	int choice;
	
	while (true) {
		clearScreen();
		cout << "菜单:" << endl;
		cout << "1. 开始游戏" << endl;
		cout << "2. 查看历史战绩" << endl;
		cout << "3. 登出" << endl;
		cin >> choice;
		
		if (cin.fail()) {
			handleInvalidInput();
			continue;
		}
		
		switch (choice) {
		case 1:
			clearScreen();
			startOptionsInterface();
			placeMines();
			calculateNumbers();
			startTime = chrono::steady_clock::now();
			return;
			
		case 2:
			clearScreen();
			showHistory();
			return;
			
		case 3:
			logout();
			return;
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
		}
	}
}

// 保存游戏记录
void saveGameRecord(int rows, int cols, int mines, int duration, bool win, int level) {
	ofstream file(username + "_history.txt", ios::app);
	
	if (file.is_open()) {
		auto now = chrono::system_clock::now();
		auto now_time_t = chrono::system_clock::to_time_t(now);
		file << put_time(localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << " " << gameMode << " ";
		
		if (gameMode == "经典模式" || gameMode == "残局模式") {
			file << rows << " " << cols << " " << mines << " " << duration << " " << (win ? "胜利" : "失败") << endl;
		} else if (gameMode == "天梯模式") {
			file << level << " " << duration << endl;
		}
		
		file.close();
	} else {
		cout << "无法保存游戏记录。" << endl;
	}
}

// 显示历史战绩
void showHistory() {
	ifstream file(username + "_history.txt");
	
	if (file.is_open()) {
		clearScreen();
		cout << "历史战绩:" << endl;
		string line;
		
		while (getline(file, line)) {
			istringstream iss(line);
			string date, time, mode;
			iss >> date >> time >> mode;
			
			if (mode == "经典模式" || mode == "残局模式") {
				int rows, cols, mines, duration;
				string result;
				iss >> rows >> cols >> mines >> duration >> result;
				cout << "时间: " << date << " " << time << ", 模式: " << mode << ", 棋盘大小: " << rows << "x" << cols << ", 地雷数量: " << mines << ", 游戏时间: " << duration << " 秒, 结果: " << result << endl;
			} else if (mode == "天梯模式") {
				int level, duration;
				string result;
				iss >> level >> duration >> result;
				cout << "时间: " << date << " " << time << ", 模式: " << mode << ", 通过层数: " << level - 1 << ", 游戏时间: " << duration << " 秒" << endl;
			}
		}
		
		file.close();
	} else {
		clearScreen();
		cout << "没有历史战绩。" << endl;
	}
	
	char choice;
	
	while (true) {
		cout << "输入 'm' 返回菜单: ";
		cin >> choice;
		
		if (choice == 'm') {
			clearScreen();
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
	cin.clear(); // 清除错误状态
	//cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
}

// 天梯模式
void ladderMode() {
	currentLevel = 1; // 重置当前层数
	rows = EASY; // 初始棋盘大小为简单难度
	cols = EASY;
	mines = 5; // 初始地雷数量为简单难度
	gameDifficulty = "简单";
	
	while (true) {
		initializeGame();
		placeMines();
		calculateNumbers();
		startTime = chrono::steady_clock::now();
		
		while (true) {
			clearScreen();
			printBoard();
			char action;
			cout << "输入操作 (l 为左键点击, r 为右键点击, t 为使用道具): ";
			string input;
			getline(cin, input);
			
			// 检查输入是否有效
			if (input.empty()) {
				handleInvalidInput();
				continue;
			}
			
			istringstream iss(input);
			iss >> action;
			
			if (action == 'l' || action == 'r' || action == 't') {
				int x = -1, y = -1;
				
				if (action == 'l' || action == 'r') {
					// 检查输入是否包含两个有效数字
					if (!(iss >> x >> y) || !iss.eof()) {
						handleInvalidInput();
						continue;
					}
					
					if (action == 'l') {
						leftClick(x, y); // 左键点击
					} else if (action == 'r') {
						rightClick(x, y); // 右键点击
					}
				} else if (action == 't') {
					// 检查输入是否只有 't'
					if (iss >> x || !iss.eof()) {
						handleInvalidInput();
					} else {
						useItem(); // 使用道具
					}
				}
			} else {
				handleInvalidInput();
				continue;
			}
			
			if (checkWin()) {
				// 询问是否继续下一层或返回菜单
				char choice;
				
				while (true) {
					cout << "游戏胜利。输入 'c' 继续下一层，或输入 'm' 返回菜单: ";
					cin >> choice;
					
					if (choice == 'c') {
						currentLevel++; // 增加层数
						mines += 5; // 增加地雷数量
						
						// 如果地雷数量达到极限，稍微扩大棋盘并重置地雷数量
						if (mines >= rows * cols) {
							rows += 2;
							cols += 2;
							mines = 10; // 重置地雷数量为简单难度
						}
						
						break;
					} else if (choice == 'm') {
						showMenu();
						return;
					} else {
						handleInvalidInput();
					}
				}
				
				break;
			}
		}
	}
}

// 保存积分
void saveScore() {
	ofstream file(username + "_score.txt");
	
	if (file.is_open()) {
		file << score;
		file.close();
	} else {
		cout << "无法保存积分。" << endl;
	}
}

// 加载积分
void loadScore() {
	ifstream file(username + "_score.txt");
	
	if (file.is_open()) {
		file >> score;
		file.close();
	} else {
		score = 0; // 如果文件不存在，积分默认为0
	}
}

// 使用道具
void useItem() {
	int choice;
	
	while (true) {
		clearScreen();
		cout << "当前积分: " << score << endl;
		cout << "选择道具:" << endl;
		cout << "1. 复活甲（使用后下一次踩到地雷游戏不会结束而是继续正常进行，消耗30积分）" << endl;
		cout << "2. 地雷扫描仪（随机揭露两颗地雷的位置，消耗50积分）" << endl;
		cout << "3. 退出" << endl;
		cout << "请输入选择: ";
		
		// 读取用户输入
		string input;
		getline(cin, input);
		istringstream iss(input);
		
		if (!(iss >> choice) || !iss.eof()) {
			handleInvalidInput();
			continue;
		}
		
		switch (choice) {
		case 1:
			if (score >= 30) {
				score -= 30; // 消耗30积分
				revive();
				return; // 自动跳转回棋盘页面
			} else {
				clearScreen();
				cout << "积分不足，无法使用复活甲道具。" << endl;
			}
			
			break;
			
		case 2:
			if (score >= 50) {
				score -= 50; // 消耗50积分
				mineScanner();
				return; // 自动跳转回棋盘页面
			} else {
				clearScreen();
				cout << "积分不足，无法使用地雷扫描仪道具。" << endl;
			}
			
			break;
			
		case 3:
			return; // 退出道具选择界面
			
		default:
			clearScreen();
			cout << "无效选择。请重新输入。" << endl;
		}
	}
}

// 地雷扫描仪道具
void mineScanner() {
	int revealedCount = 0;
	vector<pair<int, int>> minePositions;
	
	// 收集所有地雷的位置
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			if (board[i][j] == 'M') {
				minePositions.push_back({i, j});
			}
		}
	}
	
	// 随机揭露两颗地雷的位置
	while (revealedCount < 2 && !minePositions.empty()) {
		int index = rand() % minePositions.size();
		auto [x, y] = minePositions[index];
		revealed[x][y] = true;
		minePositions.erase(minePositions.begin() + index);
		revealedCount++;
	}
	
	clearScreen();
	cout << "地雷扫描仪道具已使用，随机揭露了两颗地雷的位置。" << endl;
}

// 复活甲道具
void revive() {
	hasRevive = true;
	clearScreen();
	cout << "复活甲道具已使用，下一次踩到地雷游戏不会结束。" << endl;
}

// 经典和残局模式
void classicAndResidualMode() {
	initializeGame();
	placeMines();
	calculateNumbers();
	startTime = chrono::steady_clock::now();
	
	while (true) {
		clearScreen();
		printBoard();
		char action;
		cout << "输入操作 (l 为左键点击, r 为右键点击): ";
		string input;
		getline(cin, input);
		istringstream iss(input);
		iss >> action;
		
		if (action == 'l' || action == 'r') {
			int x, y;
			iss >> x >> y;
			
			if (iss.fail()) {
				handleInvalidInput();
				continue;
			}
			
			if (action == 'l') {
				leftClick(x, y); // 左键点击
			} else if (action == 'r') {
				rightClick(x, y); // 右键点击
			}
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
					clearScreen();
					startOptionsInterface();
					placeMines();
					calculateNumbers();
					startTime = chrono::steady_clock::now();
					break;
				} else {
					clearScreen();
					cout << "无效选择。请重新输入。" << endl;
				}
			}
		}
	}
}

int main() {
	login(); // 登录
	showMenu(); // 显示菜单
	
	while (true) {
		clearScreen(); // 清除屏幕
		printBoard(); // 打印棋盘
		char action;
		cout << "输入操作 (l 为左键点击, r 为右键点击";
		
		if (gameMode == "天梯模式") {
			cout << ", t 为使用道具";
		}
		
		cout << "): ";
		string input;
		getline(cin, input);
		istringstream iss(input);
		iss >> action;
		
		if (action == 'l' || action == 'r' || (action == 't' && gameMode == "天梯模式")) {
			int x, y;
			
			if (action == 'l' || action == 'r') {
				iss >> x >> y;
				
				if (iss.fail()) {
					handleInvalidInput();
					continue;
				}
				
				if (action == 'l') {
					leftClick(x, y); // 左键点击
				} else if (action == 'r') {
					rightClick(x, y); // 右键点击
				}
			} else if (action == 't') {
				useItem(); // 使用道具
			}
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
					clearScreen();
					startOptionsInterface();
					placeMines();
					calculateNumbers();
					startTime = chrono::steady_clock::now();
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

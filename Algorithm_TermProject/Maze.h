#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>

using namespace std;

#define ROAD		0 // 길
#define WALL		1 // 벽
#define PASS		2 // 지나온 길
#define BLOCK		3 // 길 막아버리기
#define MOUSE		4 // 현재 쥐 위치
#define NOWAY		7 // 막다른 길
#define START		8 // 시작지점
#define END			9 // 도착지점
#define MAX_STACK_SIZE 10000
//********************************************************************************************
// 좌표
struct Point2D{
	int x;	// x 좌표
	int y;	// y 좌표

	Point2D(int xx=0, int yy=0)
	{
		x = xx;
		y = yy;
	}
};
//********************************************************************************************
// CMaze 클래스
class CMaze
{
public:
	int			row;			// 미로의 가로
	int			col;			// 미로의 세로
	int**		maze;			// 미로의 각 항목을 저장
	int**		map;			// 쥐의 이동 현황을 저장
	Point2D		stack[MAX_STACK_SIZE]; // 최적화된 길 배열
	Point2D		e;				// stack 배열 초기화 변수
	int			m_top;			// stack 배열 인덱스
	Point2D		m_start;		// 미로의 입구
	Point2D		m_exit;			// 미로의 출구
	Point2D		now;			// 현재 위치
	Point2D		pre;			// 바로 이전 위치
	char		tmp;			// 파일 읽을 때 사용할 변수
	ifstream	file;			// 파일 읽을 때 사용할 변수
	int			energy;			// 에너지
	double		mana;			// 마나
	int			usescan;		// 스캔을 사용한 횟수
//********************************************************************************************
	// 생성자
	CMaze(void)
	{ 
		// 변수들 초기화
		char name[30];
		row = 1;
		col = 0;
		m_start.x = 0;
		m_start.y = 1;
		mana = 0.0;
		usescan = 0;
		e.x = -1;
		e.y = -1;
		m_top = 0;

		// 최적화 배열 초기화
		for(int i = 0; i < MAX_STACK_SIZE; i++)
		{
			stack[i].x = e.x;
			stack[i].y = e.y;
		}

		// 파일 이름을 입력 받는다.
		cout << "텍스트 파일 이름을 입력하세요 : ";
		cin >> name;

		// 화면을 지운다.
		system("cls");

		// 파일을 연다.
		file.open(name);

		// 만약 파일이 없을 경우 에러 메세지를 준다.
		if(!file)
		{
			cerr << "지도를 찾을 수 없습니다!" << endl;
			getchar();
			getchar();
			exit(1);
		}

		// 미로 사이즈를 탐색한다.
		SearchSize();

		// 미로 사이즈만큼 배열을 동적으로 할당한다.
		Init(col, row);

		// 초기 에너지는 row*col*2
		energy = row*col*2;

		// 미로를 2차원 배열에 저장한다.
		StoreInArray();

		// 미로를 콘솔창에 출력한다.
		PrintMaze();

		// 행과 열을 콘솔창에 출력한다.
		PrintSize(row, col);

		// 에너지 현황을 콘솔창에 출력한다.
		PrintEnergy(energy);

		// 마나와 스캔 현황을 콘솔창에 출력한다.
		PrintMana(mana, usescan);

		getchar();
		getchar();
	}
//********************************************************************************************
	// 소멸자
	~CMaze(void)
	{ 
		// 메모리를 동적 해제한다.
		Reset(); 
	}
//********************************************************************************************
	// IsEmpty : 최적화 배열 stack이 비었는가?
	bool IsEmpty()
	{ 
		return m_top == 0; 
	}
//********************************************************************************************
	// IsFull : 최적화 배열 stack이 꽉 찼는가?
	bool IsFull()
	{ 
		return m_top == MAX_STACK_SIZE; 
	}
//********************************************************************************************
	// Push : 해당 좌표를 최적화 배열에 넣는다.
	void Push(Point2D &p)
	{
		// 배열이 꽉찼으면 에러메세지
		if(IsFull())
		{
			printf("Error : Stack Full Error\n");
			return;
		}
		m_top++; // 인덱스 1 증가
		stack[m_top] = p; // 해당 좌표 삽입
		
	}
//********************************************************************************************
	// Pop : 최적화 배열에서 좌표 하나를 삭제한다.
	void Pop()
	{
		// 배열이 비어있으면 에러메세지
		if(IsEmpty())
		{
			printf("Error : Stack Empty Error\n");
			return ;
		}
		stack[m_top] = e; // 좌표 하나를 초기화 시키고
		m_top--; // 인덱스 1 감소
	}
//********************************************************************************************
	// Peek : 가장 최근에 삽입된 좌표를 출력한다.
	Point2D Peek()
	{
		// 배열이 비어있으면 에러메세지
		if(IsEmpty())
		{
			printf("Error : Stack Empty Error\n");
			return 0;
		}
		return stack[m_top]; // 좌표 반환
	}
//********************************************************************************************
	// IsNoWay : 현재 좌표가 막다른 길인가?
	bool IsNoWay(int x, int y)
	{
		// 4개의 방향 중에서 1개를 제외하고 나머지가 벽인 경우 true
		if((maze[x][y+1] != WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] != WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] != WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] != WALL))
			return true;

		return false; // 아니면 false
	}
//********************************************************************************************
	// IsCross : 현재 좌표가 교차로인가? (갈림길인가?)
	bool IsCross(int x, int y)
	{
		// 4개의 방향 중에서 3개가 길인 경우 true
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == WALL) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == WALL) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == WALL))
			return true;

		// 4개의 방향 중에서 4개 모두 길인 경우 true
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;

		return false; // 아니면 false
	}
//********************************************************************************************
	// NearExit : 근처에 출구가 있는가?
	bool NearExit(int x, int y)
	{
		// 4개의 방향 중에서 출구가 있으면 true
		if(maze[x][y+1] == END)
			return true;
		if(maze[x+1][y] == END)
			return true;
		if(maze[x][y-1] == END)
			return true;
		if(maze[x-1][y] == END)
			return true;

		return false; // 없으면 false
	}
//********************************************************************************************
	// CanGo : 현재 좌표가 계속 진행할 수 있는 위치인가?
	bool CanGo(int x, int y)
	{
		// 4개의 방향 중에서 갈 수 있는 길이 있으면 true
		if((map[x][y+1] == ROAD) || (map[x][y-1] == ROAD) || (map[x+1][y] == ROAD) || (map[x-1][y] == ROAD))
			return true;

		return false; // 없으면 false
	}
//********************************************************************************************/
	// Init : 메모리 동적 할당
	void Init(int w, int h)
	{
		col = w;
		row = h;
		
		maze = new int*[row]; // 행에 대해서 동적 할당

		for(int i=0; i<h; i++)
			maze[i] = new int[col]; // 열에 대해서 동적 할당

		map = new int*[row]; // 행에 대해서 동적 할당

		for(int i=0; i<h; i++)
			map[i] = new int[col]; // 열에 대해서 동적 할당
	}
//********************************************************************************************
	// Reset : 메모리 동적 해제
	void Reset()
	{
		if(maze != NULL && map != NULL)
		{
			for(int i=0; i<row; i++)
				delete[] maze[i]; // 열에 대해서 동적 해제
			delete[] maze; // 행에 대해서 동적 해제

			for(int i=0; i<row; i++)
				delete[] map[i]; // 열에 대해서 동적 해제
			delete[] map; // 행에 대해서 동적 해제
		}
	}
//********************************************************************************************
	// SearchSize : 미로의 크기를 탐색한다.
	void SearchSize()
	{
		file.clear();

		while(!file.eof())
		{
			file.get(tmp); // 파일의 글자 하나를 읽어온다.

			// 읽어온 글자가 1 또는 0이면
			if(tmp == '1' || tmp == '0'){
				if(row == 1) // (열의 수가 계속 증가하는 것을 방지)
					col++; // 열의 수 1 증가
			}

			// 읽어온 글자가 \n 이면
			if(tmp == '\n'){
				row++; // 행의 수 1 증가
			}
		}
	}
//********************************************************************************************
	// StoreInArray : 미로를 배열에 저장한다.
	void StoreInArray()
	{
		file.clear();
		file.seekg(0);

		file.get(tmp); // 글자를 읽어 온다.

		for(int i = 0; i < row; i++)
		{
			for(int j = 0; j < col; j++)
			{
				if(tmp == '0')	// 글자가 0 이면
					maze[i][j] = ROAD; // 길
				else if(tmp == '1') // 글자가 1 이면
					maze[i][j] = WALL; // 벽
				else if(tmp == ' ') // 글자가 공백이면
					j--; // j--/

				do
				{
					file.get(tmp);
				}while(tmp == '\n' && !file.eof());
			}
		}

		//*** 출구 좌표를 찾는다. 
		// (입구와 출구는 테두리 벽들 중 유일하게 길)
		for(int i = 0; i < col; i++){
			// 맨 위 테두리 중에서 길이 있으면
			if(maze[0][i] == ROAD){ 
				maze[0][i] = END; // 그것은 출구다.
				m_exit.x = 0; // 출구 x좌표
				m_exit.y = i; // 출구 y좌표
			}
			// 맨 아래 테두리 중에서 길이 있으면
			else if(maze[row-1][i] == ROAD){ 
				maze[row-1][i] = END; // 그것은 출구다.
				m_exit.x = row-1; // 출구 x좌표
				m_exit.y = i; // 출구 y좌표
			}
		}

		for(int i = 0; i < row; i++){
			// 맨 왼쪽 테두리 중에서 길이 있으면
			if(maze[i][0] == ROAD){
				maze[i][0] = END; // 그것은 출구다.
				m_exit.x = i; // 출구 x좌표
				m_exit.y = 0; // 출구 y좌표
			}
			// 맨 오른쪽 테두리 중에서 길이 있으면
			else if(maze[i][col-1] == ROAD){
				maze[i][col-1] = END; // 그것은 출구다.
				m_exit.x = i; // 출구 x좌표
				m_exit.y = col-1; // 출구 y좌표
			}
		}
		maze[0][1] = START; // 입구는 무조건 (0,1)

		// maze배열을 map배열에 복사
		for(int i = 0; i < row; i++)
			for(int j = 0; j < col; j++)
			{
				if(maze[i][j] == ROAD || maze[i][j] == START || maze[i][j] == END)
					map[i][j] = ROAD;
				else if(maze[i][j] == WALL)
					map[i][j] = WALL;
			}
	}
//********************************************************************************************
	// PrintMaze : 미로를 콘솔창에 출력한다.
	void PrintMaze()
	{
		int i, j; // 인덱스

		for(i = 0; i < row; i++)
		{
			for(j = 0; j < col; j++)
			{
				if(maze[i][j] == WALL) // 해당 좌표가 벽이면
				{
					Color(2); // 색 설정 : 어두운 초록색
					cout << "■"; // 벽
				}
				else if(maze[i][j] == ROAD) // 해당 좌표가 길이면
				{
					cout << "　"; // 길
				}
				else if(maze[i][j] == START) // 해당 좌표가 시작점이면
				{
					Color(14); // 색 설정 : 노란색
					cout << "ⓢ"; // 시작점
				}
				else if(maze[i][j] == END) // 해당 좌표가 도착점이면
				{
					Color(12); // 색 설정 : 빨간색
					cout << "¶"; // 도착점
				}
			}
			cout << endl;
		}
		
		Go(col*2+2, 2); // 좌표 이동
		Color(12); // 색 설정 : 빨간색
		cout << "2016년도 1학기 알고리즘 Term Project";

		Color(15); // 색 설정 : 하얀색
		Go(col*2+2, 30); // 좌표 이동
		cout << "<제작>";
		Go(col*2+2, 31); // 좌표 이동
		cout << "2012136132 최강석";
		Go(col*2+2, 32); // 좌표 이동
		cout << "2012136139 최형근";
		Go(col*2+2, 33); // 좌표 이동
		cout << "2012136142 허준석";
	}
//********************************************************************************************
	// PrintSize : 행과 열 사이즈를 콘솔창에 출력한다.
	void PrintSize(int row, int col)
	{
		Color(14); // 색 설정 : 노란색
		Go(col*2+2, 7); // 좌표 이동
		cout << "[행] : " << row;
		Go(col*2+2, 8); // 좌표 이동
		cout << "[열] : " << col;
	}
//********************************************************************************************
	// PrintEnergy : 에너지를 콘솔창에 출력한다.
	void PrintEnergy(int e)
	{
		Color(10); // 색 설정 : 연두색
		Go(col*2+2, 10); // 좌표 이동
		cout << "[에너지] : " << e;
	}
//********************************************************************************************
	// PrintMana : 마나와 스캔을 콘솔창에 출력한다.
	void PrintMana(double m, int s)
	{
		Color(11); // 색 설정 : 하늘색
		Go(col*2+2, 12); // 좌표 이동
		printf("　[마나] : %.1f", m);

		Color(7); // 색 설정 : 회색
		Go(col*2+2, 14); // 좌표 이동
		cout << "　[스캔] : " << s;
	}
//********************************************************************************************
	// DrawMouse : 해당 좌표에 쥐를 그린다.
	void DrawMouse(Point2D p)
	{
		Go(p.y*2, p.x); // 좌표 이동
		Color(7); // 색 설정 : 회색
		cout << "ⓜ";
	}
//********************************************************************************************
	// EraseMouse : 해당 좌표에 있는 쥐를 지운다.
	void EraseMouse(Point2D p)
	{
		Go(p.y*2, p.x); // 좌표 이동
		Color(7); // 색 설정 : 회색
		cout << "　";
	}
//********************************************************************************************
	// OptimalMouse : 최적화된 길을 그린다.
	void OptimalMouse(Point2D p)
	{
		Go(p.y*2, p.x); // 좌표 이동
		Color(14); // 색 설정 : 노란색
		cout << "▦";
	}
//********************************************************************************************
	// SearchExit : 미로 찾기를 수행한다.
	void SearchExit()
	{
		now = m_start; // 현재 좌표 = 시작 지점
		DrawMouse(now); // 현재 좌표에 쥐를 그린다.
		Push(now); // 현재 좌표를 배열에 추가한다.
		map[now.x][now.y] = PASS; // 현재 좌표는 갔던 길이다.

		PrintEnergy(energy); // 에너지 출력
		PrintMana(mana, usescan); // 마나&스캔 출력

		while(1)
		{
			Point2D right = now;
			Point2D left = now;
			Point2D up = now;
			Point2D down = now;

			right.y = now.y + 1; // 현재 좌표의 오른쪽
			left.y = now.y - 1; // 현재 좌표의 왼쪽
			up.x = now.x - 1; // 현재 좌표의 위쪽
			down.x = now.x + 1; // 현재 좌표의 아래쪽

			pre = now; // 이전 위치 = 현재 위치
			EraseMouse(pre); // 이전 위치의 쥐를 지운다.

			// 미로 탐색 순서 : 오른쪽 - 아래쪽 - 왼쪽 - 위쪽
			if(map[right.x][right.y] == ROAD) // 오른쪽이 길이면
			{
				// 현재 좌표를 오른쪽 좌표로 이동
				now.x = right.x;
				now.y = right.y;
			}
			else if(map[down.x][down.y] == ROAD) // 아래쪽이 길이면
			{
				// 현재 좌표를 아래쪽 좌표로 이동
				now.x = down.x;
				now.y = down.y;
			}	
			else if(map[left.x][left.y] == ROAD) // 왼쪽이 길이면
			{
				// 현재 좌표를 왼쪽 좌표로 이동
				now.x = left.x;
				now.y = left.y;
			}
			else if(map[up.x][up.y] == ROAD) // 위쪽이 길이면
			{
				// 현재 좌표를 위쪽 좌표로 이동
				now.x = up.x;
				now.y = up.y;
			}

			DrawMouse(now); // 현재 좌표에 쥐를 그린다.
			Push(now); // 현재 좌표를 배열에 추가한다.

			energy--; // 이동 했으므로 에너지 1 감소
			PrintEnergy(energy); // 에너지 출력
			mana = mana + 0.1; // 이동 했으므로 마나 0.1 증가
			if(mana >= 0.9) // 마나가 1이 되면
			{
				mana = 0.0; // 마나를 0으로 초기화 하고
				Scan(ROAD); // 스캔 발동
			}
			PrintMana(mana, usescan); // 마나&스캔 출력

			map[now.x][now.y] = PASS; // 현재 좌표는 갔던 길이다.

			if(IsNoWay(now.x, now.y)) // 만약 현재 좌표가 막다른 길이면
			{
				GoBack(); // 되돌아가는 함수 호출
			}

			Sleep(50); // 딜레이 0.1초

			if(NearExit(now.x, now.y)) // 근처에 도착지점이 있다면?
				break; // 반복문 탈출!!
		}

		PrintOptimal(stack); // 최적화된 길을 출력
		PrintResult(); // 최종 결과 출력
	}
//********************************************************************************************
	// GoBack : 막다른 길에서 도착했을 때 갈림길로 다시 돌아간다.
	void GoBack()
	{
		while(1)
		{
			EraseMouse(now); // 현재 좌표의 쥐를 지운다.
			map[now.x][now.y] = WALL; // 현재 좌표를 벽으로 막아버린다.
			Pop(); // 배열에서 현재 좌표를 삭제한다.

			// 만약 현재 좌표가 계속 진행할 수 있는 길이면
			if(IsCross(now.x, now.y) && CanGo(now.x, now.y))
			{
				map[now.x][now.y] = PASS; // 현재 좌표는 갔던 길로 변경
				Push(now); // 다시 현재 좌표를 배열에 추가한다.

				if(CanGo(now.x, now.y))
					break; // 반복문 탈출!!
			}

			now = Peek(); // 현재 좌표는 가장 최근에 삽입된 좌표
			DrawMouse(now); // 현재 좌표에 쥐를 그린다.

			energy--; // 에너지 1 감소
			PrintEnergy(energy); // 에너지 출력
			mana = mana + 0.1; // 마나 0.1 증가
			if(mana >= 0.9) // 마나가 1이 되면
			{
				mana = 0.0; // 마나를 0으로 초기화
				Scan(PASS); // 스캔 발동
			}
			PrintMana(mana, usescan); // 마나&스캔 출력

			Sleep(50); // 딜레이
		}
	}
//********************************************************************************************
	// PrintOptimal : 최적화 배열의 내용을 출력한다.
	void PrintOptimal(Point2D p[])
	{
		for(int i = 1; i < m_top; i++)
			OptimalMouse(p[i]);
	}
//********************************************************************************************
	// PrintResult : 최종 결과를 출력한다. (총 에너지 사용량, 총 스캔 사용량)
	void PrintResult()
	{
		Color(10);
		Go(col*2+2, 18);
		cout << "********** 결과 **********";
		Go(col*2+2, 19);
		cout << "*                        *";
		Go(col*2+2, 20);
		printf("* 총 에너지 사용량 : %3d *", row*col*2 - energy);
		Go(col*2+2, 21);
		printf("* 총 　스캔 사용량 : %3d *", usescan);
		Go(col*2+2, 22);
		cout << "*                        *";
		Go(col*2+2, 23);
		cout << "**************************";

		Go(col*2+2, 25);
		Color(13);
		cout << "- Enter를 누르면 종료합니다. -";

		Go(0, row+1);

		getchar();
	}
//********************************************************************************************
	// Scan : 스캔
	void Scan(int x)
	{
		usescan++; // 스캔 사용 횟수 1 증가

		Point2D right = now;
		Point2D left = now;
		Point2D up = now;
		Point2D down = now;

		right.y = now.y + 1; // 오른쪽
		left.y = now.y - 1; // 왼쪽
		up.x = now.x - 1; // 위쪽
		down.x = now.x + 1; // 아래쪽

		// 스캔 발동 순서 : 오른쪽 - 아래쪽 - 왼쪽 - 위쪽
		if(map[right.x][right.y] == x)
		{
			// 오른쪽 스캔
			Scanning(right.x, right.y + 1 );
		}
		else if(map[down.x][down.y] == x)
		{
			// 아래쪽 스캔
			Scanning(down.x + 1, down.y);
		}	
		else if(map[left.x][left.y] == x)
		{
			// 왼쪽 스캔
			Scanning(left.x, left.y - 1);
		}
		else if(map[up.x][up.y] == x)
		{
			// 위쪽 스캔
			Scanning(up.x - 1, up.y);
		}
	}
//********************************************************************************************
	// Scanning : 콘솔 창에 스캔 발동 화면 출력 + 막다른 길을 벽으로 막기
	void Scanning(int x, int y)
	{
		for(int i = 0; i < 9; i++)
		{
			DrawScan(x, y); // 좌표에 스캔 출력
			DrawScan(x, y-1); // 좌표에 스캔 출력
			DrawScan(x+1, y-1); // 좌표에 스캔 출력
			DrawScan(x+1, y); // 좌표에 스캔 출력
			DrawScan(x+1, y+1); // 좌표에 스캔 출력
			DrawScan(x, y+1); // 좌표에 스캔 출력
			DrawScan(x-1, y+1); // 좌표에 스캔 출력
			DrawScan(x-1, y); // 좌표에 스캔 출력
			DrawScan(x-1, y-1); // 좌표에 스캔 출력
			Sleep(50); // 딜레이
			DrawBack(x, y); // 다시 복원
			DrawBack(x, y-1); // 다시 복원
			DrawBack(x+1, y-1); // 다시 복원
			DrawBack(x+1, y); // 다시 복원
			DrawBack(x+1, y+1); // 다시 복원
			DrawBack(x, y+1); // 다시 복원
			DrawBack(x-1, y+1); // 다시 복원
			DrawBack(x-1, y); // 다시 복원
			DrawBack(x-1, y-1); // 다시 복원
		}
	}
//********************************************************************************************
	// DrawScan : 해당 좌표에 스캔을 그린다.
	void DrawScan(int x, int y)
	{
		Go(y*2, x);
		Color(11);
		cout << "⊙";
	}
//********************************************************************************************
	// DrawBack : 스캔을 지우고 다시 길,벽을 그린다.
	void DrawBack(int x, int y)
	{
		Go(y*2, x);
		if(maze[x][y] == WALL) // 해당 좌표가 벽이었으면
		{
			Color(2);
			cout << "■"; // 다시 벽을 그린다.
		}
		else if(maze[x][y] == ROAD) // 해당 좌표가 길이었으면
		{
			if(IsNoWay(x, y)) // 막다른 길이면
			{
				maze[x][y] = WALL; // 벽으로 변경
				map[x][y] = WALL; // 벽으로 변경
			}
			else // 막다른 길이 아니면
				cout << "　"; // 다시 길을 그린다.
		}
		else if(maze[x][y] == START) // 해당 좌표가 시작점이었으면
		{
			Color(14);
			cout << "ⓢ"; // 다시 시작점을 그린다.
		}
		else if(maze[x][y] == END) // 해당 좌표가 도착점이었으면
		{
			Color(12);
			cout << "¶"; // 다시 도착점을 그린다.
		}
	}
//********************************************************************************************
	// Go : 콘솔창에서 해당 좌표로 이동한다. (WinAPI)
	void Go(int x, int y) 
	{
		COORD pos = {x, y};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	}
//********************************************************************************************
	// Color : 콘솔창에 글자 색을 변경한다. (WinAPI)/
	void Color(unsigned short color) 
	{
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hCon,color);
	}
};
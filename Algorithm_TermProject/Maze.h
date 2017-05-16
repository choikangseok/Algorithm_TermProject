#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>

using namespace std;

#define ROAD		0 // ��
#define WALL		1 // ��
#define PASS		2 // ������ ��
#define BLOCK		3 // �� ���ƹ�����
#define MOUSE		4 // ���� �� ��ġ
#define NOWAY		7 // ���ٸ� ��
#define START		8 // ��������
#define END			9 // ��������
#define MAX_STACK_SIZE 10000
//********************************************************************************************
// ��ǥ
struct Point2D{
	int x;	// x ��ǥ
	int y;	// y ��ǥ

	Point2D(int xx=0, int yy=0)
	{
		x = xx;
		y = yy;
	}
};
//********************************************************************************************
// CMaze Ŭ����
class CMaze
{
public:
	int			row;			// �̷��� ����
	int			col;			// �̷��� ����
	int**		maze;			// �̷��� �� �׸��� ����
	int**		map;			// ���� �̵� ��Ȳ�� ����
	Point2D		stack[MAX_STACK_SIZE]; // ����ȭ�� �� �迭
	Point2D		e;				// stack �迭 �ʱ�ȭ ����
	int			m_top;			// stack �迭 �ε���
	Point2D		m_start;		// �̷��� �Ա�
	Point2D		m_exit;			// �̷��� �ⱸ
	Point2D		now;			// ���� ��ġ
	Point2D		pre;			// �ٷ� ���� ��ġ
	char		tmp;			// ���� ���� �� ����� ����
	ifstream	file;			// ���� ���� �� ����� ����
	int			energy;			// ������
	double		mana;			// ����
	int			usescan;		// ��ĵ�� ����� Ƚ��
//********************************************************************************************
	// ������
	CMaze(void)
	{ 
		// ������ �ʱ�ȭ
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

		// ����ȭ �迭 �ʱ�ȭ
		for(int i = 0; i < MAX_STACK_SIZE; i++)
		{
			stack[i].x = e.x;
			stack[i].y = e.y;
		}

		// ���� �̸��� �Է� �޴´�.
		cout << "�ؽ�Ʈ ���� �̸��� �Է��ϼ��� : ";
		cin >> name;

		// ȭ���� �����.
		system("cls");

		// ������ ����.
		file.open(name);

		// ���� ������ ���� ��� ���� �޼����� �ش�.
		if(!file)
		{
			cerr << "������ ã�� �� �����ϴ�!" << endl;
			getchar();
			getchar();
			exit(1);
		}

		// �̷� ����� Ž���Ѵ�.
		SearchSize();

		// �̷� �����ŭ �迭�� �������� �Ҵ��Ѵ�.
		Init(col, row);

		// �ʱ� �������� row*col*2
		energy = row*col*2;

		// �̷θ� 2���� �迭�� �����Ѵ�.
		StoreInArray();

		// �̷θ� �ܼ�â�� ����Ѵ�.
		PrintMaze();

		// ��� ���� �ܼ�â�� ����Ѵ�.
		PrintSize(row, col);

		// ������ ��Ȳ�� �ܼ�â�� ����Ѵ�.
		PrintEnergy(energy);

		// ������ ��ĵ ��Ȳ�� �ܼ�â�� ����Ѵ�.
		PrintMana(mana, usescan);

		getchar();
		getchar();
	}
//********************************************************************************************
	// �Ҹ���
	~CMaze(void)
	{ 
		// �޸𸮸� ���� �����Ѵ�.
		Reset(); 
	}
//********************************************************************************************
	// IsEmpty : ����ȭ �迭 stack�� ����°�?
	bool IsEmpty()
	{ 
		return m_top == 0; 
	}
//********************************************************************************************
	// IsFull : ����ȭ �迭 stack�� �� á�°�?
	bool IsFull()
	{ 
		return m_top == MAX_STACK_SIZE; 
	}
//********************************************************************************************
	// Push : �ش� ��ǥ�� ����ȭ �迭�� �ִ´�.
	void Push(Point2D &p)
	{
		// �迭�� ��á���� �����޼���
		if(IsFull())
		{
			printf("Error : Stack Full Error\n");
			return;
		}
		m_top++; // �ε��� 1 ����
		stack[m_top] = p; // �ش� ��ǥ ����
		
	}
//********************************************************************************************
	// Pop : ����ȭ �迭���� ��ǥ �ϳ��� �����Ѵ�.
	void Pop()
	{
		// �迭�� ��������� �����޼���
		if(IsEmpty())
		{
			printf("Error : Stack Empty Error\n");
			return ;
		}
		stack[m_top] = e; // ��ǥ �ϳ��� �ʱ�ȭ ��Ű��
		m_top--; // �ε��� 1 ����
	}
//********************************************************************************************
	// Peek : ���� �ֱٿ� ���Ե� ��ǥ�� ����Ѵ�.
	Point2D Peek()
	{
		// �迭�� ��������� �����޼���
		if(IsEmpty())
		{
			printf("Error : Stack Empty Error\n");
			return 0;
		}
		return stack[m_top]; // ��ǥ ��ȯ
	}
//********************************************************************************************
	// IsNoWay : ���� ��ǥ�� ���ٸ� ���ΰ�?
	bool IsNoWay(int x, int y)
	{
		// 4���� ���� �߿��� 1���� �����ϰ� �������� ���� ��� true
		if((maze[x][y+1] != WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] != WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] != WALL) && (maze[x-1][y] == WALL))
			return true;
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == WALL) && (maze[x+1][y] == WALL) && (maze[x-1][y] != WALL))
			return true;

		return false; // �ƴϸ� false
	}
//********************************************************************************************
	// IsCross : ���� ��ǥ�� �������ΰ�? (�������ΰ�?)
	bool IsCross(int x, int y)
	{
		// 4���� ���� �߿��� 3���� ���� ��� true
		if((maze[x][y+1] == WALL) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == WALL) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == WALL) && (maze[x-1][y] == ROAD))
			return true;
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == WALL))
			return true;

		// 4���� ���� �߿��� 4�� ��� ���� ��� true
		if((maze[x][y+1] == ROAD) && (maze[x][y-1] == ROAD) && (maze[x+1][y] == ROAD) && (maze[x-1][y] == ROAD))
			return true;

		return false; // �ƴϸ� false
	}
//********************************************************************************************
	// NearExit : ��ó�� �ⱸ�� �ִ°�?
	bool NearExit(int x, int y)
	{
		// 4���� ���� �߿��� �ⱸ�� ������ true
		if(maze[x][y+1] == END)
			return true;
		if(maze[x+1][y] == END)
			return true;
		if(maze[x][y-1] == END)
			return true;
		if(maze[x-1][y] == END)
			return true;

		return false; // ������ false
	}
//********************************************************************************************
	// CanGo : ���� ��ǥ�� ��� ������ �� �ִ� ��ġ�ΰ�?
	bool CanGo(int x, int y)
	{
		// 4���� ���� �߿��� �� �� �ִ� ���� ������ true
		if((map[x][y+1] == ROAD) || (map[x][y-1] == ROAD) || (map[x+1][y] == ROAD) || (map[x-1][y] == ROAD))
			return true;

		return false; // ������ false
	}
//********************************************************************************************/
	// Init : �޸� ���� �Ҵ�
	void Init(int w, int h)
	{
		col = w;
		row = h;
		
		maze = new int*[row]; // �࿡ ���ؼ� ���� �Ҵ�

		for(int i=0; i<h; i++)
			maze[i] = new int[col]; // ���� ���ؼ� ���� �Ҵ�

		map = new int*[row]; // �࿡ ���ؼ� ���� �Ҵ�

		for(int i=0; i<h; i++)
			map[i] = new int[col]; // ���� ���ؼ� ���� �Ҵ�
	}
//********************************************************************************************
	// Reset : �޸� ���� ����
	void Reset()
	{
		if(maze != NULL && map != NULL)
		{
			for(int i=0; i<row; i++)
				delete[] maze[i]; // ���� ���ؼ� ���� ����
			delete[] maze; // �࿡ ���ؼ� ���� ����

			for(int i=0; i<row; i++)
				delete[] map[i]; // ���� ���ؼ� ���� ����
			delete[] map; // �࿡ ���ؼ� ���� ����
		}
	}
//********************************************************************************************
	// SearchSize : �̷��� ũ�⸦ Ž���Ѵ�.
	void SearchSize()
	{
		file.clear();

		while(!file.eof())
		{
			file.get(tmp); // ������ ���� �ϳ��� �о�´�.

			// �о�� ���ڰ� 1 �Ǵ� 0�̸�
			if(tmp == '1' || tmp == '0'){
				if(row == 1) // (���� ���� ��� �����ϴ� ���� ����)
					col++; // ���� �� 1 ����
			}

			// �о�� ���ڰ� \n �̸�
			if(tmp == '\n'){
				row++; // ���� �� 1 ����
			}
		}
	}
//********************************************************************************************
	// StoreInArray : �̷θ� �迭�� �����Ѵ�.
	void StoreInArray()
	{
		file.clear();
		file.seekg(0);

		file.get(tmp); // ���ڸ� �о� �´�.

		for(int i = 0; i < row; i++)
		{
			for(int j = 0; j < col; j++)
			{
				if(tmp == '0')	// ���ڰ� 0 �̸�
					maze[i][j] = ROAD; // ��
				else if(tmp == '1') // ���ڰ� 1 �̸�
					maze[i][j] = WALL; // ��
				else if(tmp == ' ') // ���ڰ� �����̸�
					j--; // j--/

				do
				{
					file.get(tmp);
				}while(tmp == '\n' && !file.eof());
			}
		}

		//*** �ⱸ ��ǥ�� ã�´�. 
		// (�Ա��� �ⱸ�� �׵θ� ���� �� �����ϰ� ��)
		for(int i = 0; i < col; i++){
			// �� �� �׵θ� �߿��� ���� ������
			if(maze[0][i] == ROAD){ 
				maze[0][i] = END; // �װ��� �ⱸ��.
				m_exit.x = 0; // �ⱸ x��ǥ
				m_exit.y = i; // �ⱸ y��ǥ
			}
			// �� �Ʒ� �׵θ� �߿��� ���� ������
			else if(maze[row-1][i] == ROAD){ 
				maze[row-1][i] = END; // �װ��� �ⱸ��.
				m_exit.x = row-1; // �ⱸ x��ǥ
				m_exit.y = i; // �ⱸ y��ǥ
			}
		}

		for(int i = 0; i < row; i++){
			// �� ���� �׵θ� �߿��� ���� ������
			if(maze[i][0] == ROAD){
				maze[i][0] = END; // �װ��� �ⱸ��.
				m_exit.x = i; // �ⱸ x��ǥ
				m_exit.y = 0; // �ⱸ y��ǥ
			}
			// �� ������ �׵θ� �߿��� ���� ������
			else if(maze[i][col-1] == ROAD){
				maze[i][col-1] = END; // �װ��� �ⱸ��.
				m_exit.x = i; // �ⱸ x��ǥ
				m_exit.y = col-1; // �ⱸ y��ǥ
			}
		}
		maze[0][1] = START; // �Ա��� ������ (0,1)

		// maze�迭�� map�迭�� ����
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
	// PrintMaze : �̷θ� �ܼ�â�� ����Ѵ�.
	void PrintMaze()
	{
		int i, j; // �ε���

		for(i = 0; i < row; i++)
		{
			for(j = 0; j < col; j++)
			{
				if(maze[i][j] == WALL) // �ش� ��ǥ�� ���̸�
				{
					Color(2); // �� ���� : ��ο� �ʷϻ�
					cout << "��"; // ��
				}
				else if(maze[i][j] == ROAD) // �ش� ��ǥ�� ���̸�
				{
					cout << "��"; // ��
				}
				else if(maze[i][j] == START) // �ش� ��ǥ�� �������̸�
				{
					Color(14); // �� ���� : �����
					cout << "��"; // ������
				}
				else if(maze[i][j] == END) // �ش� ��ǥ�� �������̸�
				{
					Color(12); // �� ���� : ������
					cout << "��"; // ������
				}
			}
			cout << endl;
		}
		
		Go(col*2+2, 2); // ��ǥ �̵�
		Color(12); // �� ���� : ������
		cout << "2016�⵵ 1�б� �˰��� Term Project";

		Color(15); // �� ���� : �Ͼ��
		Go(col*2+2, 30); // ��ǥ �̵�
		cout << "<����>";
		Go(col*2+2, 31); // ��ǥ �̵�
		cout << "2012136132 �ְ���";
		Go(col*2+2, 32); // ��ǥ �̵�
		cout << "2012136139 ������";
		Go(col*2+2, 33); // ��ǥ �̵�
		cout << "2012136142 ���ؼ�";
	}
//********************************************************************************************
	// PrintSize : ��� �� ����� �ܼ�â�� ����Ѵ�.
	void PrintSize(int row, int col)
	{
		Color(14); // �� ���� : �����
		Go(col*2+2, 7); // ��ǥ �̵�
		cout << "[��] : " << row;
		Go(col*2+2, 8); // ��ǥ �̵�
		cout << "[��] : " << col;
	}
//********************************************************************************************
	// PrintEnergy : �������� �ܼ�â�� ����Ѵ�.
	void PrintEnergy(int e)
	{
		Color(10); // �� ���� : ���λ�
		Go(col*2+2, 10); // ��ǥ �̵�
		cout << "[������] : " << e;
	}
//********************************************************************************************
	// PrintMana : ������ ��ĵ�� �ܼ�â�� ����Ѵ�.
	void PrintMana(double m, int s)
	{
		Color(11); // �� ���� : �ϴû�
		Go(col*2+2, 12); // ��ǥ �̵�
		printf("��[����] : %.1f", m);

		Color(7); // �� ���� : ȸ��
		Go(col*2+2, 14); // ��ǥ �̵�
		cout << "��[��ĵ] : " << s;
	}
//********************************************************************************************
	// DrawMouse : �ش� ��ǥ�� �㸦 �׸���.
	void DrawMouse(Point2D p)
	{
		Go(p.y*2, p.x); // ��ǥ �̵�
		Color(7); // �� ���� : ȸ��
		cout << "��";
	}
//********************************************************************************************
	// EraseMouse : �ش� ��ǥ�� �ִ� �㸦 �����.
	void EraseMouse(Point2D p)
	{
		Go(p.y*2, p.x); // ��ǥ �̵�
		Color(7); // �� ���� : ȸ��
		cout << "��";
	}
//********************************************************************************************
	// OptimalMouse : ����ȭ�� ���� �׸���.
	void OptimalMouse(Point2D p)
	{
		Go(p.y*2, p.x); // ��ǥ �̵�
		Color(14); // �� ���� : �����
		cout << "��";
	}
//********************************************************************************************
	// SearchExit : �̷� ã�⸦ �����Ѵ�.
	void SearchExit()
	{
		now = m_start; // ���� ��ǥ = ���� ����
		DrawMouse(now); // ���� ��ǥ�� �㸦 �׸���.
		Push(now); // ���� ��ǥ�� �迭�� �߰��Ѵ�.
		map[now.x][now.y] = PASS; // ���� ��ǥ�� ���� ���̴�.

		PrintEnergy(energy); // ������ ���
		PrintMana(mana, usescan); // ����&��ĵ ���

		while(1)
		{
			Point2D right = now;
			Point2D left = now;
			Point2D up = now;
			Point2D down = now;

			right.y = now.y + 1; // ���� ��ǥ�� ������
			left.y = now.y - 1; // ���� ��ǥ�� ����
			up.x = now.x - 1; // ���� ��ǥ�� ����
			down.x = now.x + 1; // ���� ��ǥ�� �Ʒ���

			pre = now; // ���� ��ġ = ���� ��ġ
			EraseMouse(pre); // ���� ��ġ�� �㸦 �����.

			// �̷� Ž�� ���� : ������ - �Ʒ��� - ���� - ����
			if(map[right.x][right.y] == ROAD) // �������� ���̸�
			{
				// ���� ��ǥ�� ������ ��ǥ�� �̵�
				now.x = right.x;
				now.y = right.y;
			}
			else if(map[down.x][down.y] == ROAD) // �Ʒ����� ���̸�
			{
				// ���� ��ǥ�� �Ʒ��� ��ǥ�� �̵�
				now.x = down.x;
				now.y = down.y;
			}	
			else if(map[left.x][left.y] == ROAD) // ������ ���̸�
			{
				// ���� ��ǥ�� ���� ��ǥ�� �̵�
				now.x = left.x;
				now.y = left.y;
			}
			else if(map[up.x][up.y] == ROAD) // ������ ���̸�
			{
				// ���� ��ǥ�� ���� ��ǥ�� �̵�
				now.x = up.x;
				now.y = up.y;
			}

			DrawMouse(now); // ���� ��ǥ�� �㸦 �׸���.
			Push(now); // ���� ��ǥ�� �迭�� �߰��Ѵ�.

			energy--; // �̵� �����Ƿ� ������ 1 ����
			PrintEnergy(energy); // ������ ���
			mana = mana + 0.1; // �̵� �����Ƿ� ���� 0.1 ����
			if(mana >= 0.9) // ������ 1�� �Ǹ�
			{
				mana = 0.0; // ������ 0���� �ʱ�ȭ �ϰ�
				Scan(ROAD); // ��ĵ �ߵ�
			}
			PrintMana(mana, usescan); // ����&��ĵ ���

			map[now.x][now.y] = PASS; // ���� ��ǥ�� ���� ���̴�.

			if(IsNoWay(now.x, now.y)) // ���� ���� ��ǥ�� ���ٸ� ���̸�
			{
				GoBack(); // �ǵ��ư��� �Լ� ȣ��
			}

			Sleep(50); // ������ 0.1��

			if(NearExit(now.x, now.y)) // ��ó�� ���������� �ִٸ�?
				break; // �ݺ��� Ż��!!
		}

		PrintOptimal(stack); // ����ȭ�� ���� ���
		PrintResult(); // ���� ��� ���
	}
//********************************************************************************************
	// GoBack : ���ٸ� �濡�� �������� �� ������� �ٽ� ���ư���.
	void GoBack()
	{
		while(1)
		{
			EraseMouse(now); // ���� ��ǥ�� �㸦 �����.
			map[now.x][now.y] = WALL; // ���� ��ǥ�� ������ ���ƹ�����.
			Pop(); // �迭���� ���� ��ǥ�� �����Ѵ�.

			// ���� ���� ��ǥ�� ��� ������ �� �ִ� ���̸�
			if(IsCross(now.x, now.y) && CanGo(now.x, now.y))
			{
				map[now.x][now.y] = PASS; // ���� ��ǥ�� ���� ��� ����
				Push(now); // �ٽ� ���� ��ǥ�� �迭�� �߰��Ѵ�.

				if(CanGo(now.x, now.y))
					break; // �ݺ��� Ż��!!
			}

			now = Peek(); // ���� ��ǥ�� ���� �ֱٿ� ���Ե� ��ǥ
			DrawMouse(now); // ���� ��ǥ�� �㸦 �׸���.

			energy--; // ������ 1 ����
			PrintEnergy(energy); // ������ ���
			mana = mana + 0.1; // ���� 0.1 ����
			if(mana >= 0.9) // ������ 1�� �Ǹ�
			{
				mana = 0.0; // ������ 0���� �ʱ�ȭ
				Scan(PASS); // ��ĵ �ߵ�
			}
			PrintMana(mana, usescan); // ����&��ĵ ���

			Sleep(50); // ������
		}
	}
//********************************************************************************************
	// PrintOptimal : ����ȭ �迭�� ������ ����Ѵ�.
	void PrintOptimal(Point2D p[])
	{
		for(int i = 1; i < m_top; i++)
			OptimalMouse(p[i]);
	}
//********************************************************************************************
	// PrintResult : ���� ����� ����Ѵ�. (�� ������ ��뷮, �� ��ĵ ��뷮)
	void PrintResult()
	{
		Color(10);
		Go(col*2+2, 18);
		cout << "********** ��� **********";
		Go(col*2+2, 19);
		cout << "*                        *";
		Go(col*2+2, 20);
		printf("* �� ������ ��뷮 : %3d *", row*col*2 - energy);
		Go(col*2+2, 21);
		printf("* �� ����ĵ ��뷮 : %3d *", usescan);
		Go(col*2+2, 22);
		cout << "*                        *";
		Go(col*2+2, 23);
		cout << "**************************";

		Go(col*2+2, 25);
		Color(13);
		cout << "- Enter�� ������ �����մϴ�. -";

		Go(0, row+1);

		getchar();
	}
//********************************************************************************************
	// Scan : ��ĵ
	void Scan(int x)
	{
		usescan++; // ��ĵ ��� Ƚ�� 1 ����

		Point2D right = now;
		Point2D left = now;
		Point2D up = now;
		Point2D down = now;

		right.y = now.y + 1; // ������
		left.y = now.y - 1; // ����
		up.x = now.x - 1; // ����
		down.x = now.x + 1; // �Ʒ���

		// ��ĵ �ߵ� ���� : ������ - �Ʒ��� - ���� - ����
		if(map[right.x][right.y] == x)
		{
			// ������ ��ĵ
			Scanning(right.x, right.y + 1 );
		}
		else if(map[down.x][down.y] == x)
		{
			// �Ʒ��� ��ĵ
			Scanning(down.x + 1, down.y);
		}	
		else if(map[left.x][left.y] == x)
		{
			// ���� ��ĵ
			Scanning(left.x, left.y - 1);
		}
		else if(map[up.x][up.y] == x)
		{
			// ���� ��ĵ
			Scanning(up.x - 1, up.y);
		}
	}
//********************************************************************************************
	// Scanning : �ܼ� â�� ��ĵ �ߵ� ȭ�� ��� + ���ٸ� ���� ������ ����
	void Scanning(int x, int y)
	{
		for(int i = 0; i < 9; i++)
		{
			DrawScan(x, y); // ��ǥ�� ��ĵ ���
			DrawScan(x, y-1); // ��ǥ�� ��ĵ ���
			DrawScan(x+1, y-1); // ��ǥ�� ��ĵ ���
			DrawScan(x+1, y); // ��ǥ�� ��ĵ ���
			DrawScan(x+1, y+1); // ��ǥ�� ��ĵ ���
			DrawScan(x, y+1); // ��ǥ�� ��ĵ ���
			DrawScan(x-1, y+1); // ��ǥ�� ��ĵ ���
			DrawScan(x-1, y); // ��ǥ�� ��ĵ ���
			DrawScan(x-1, y-1); // ��ǥ�� ��ĵ ���
			Sleep(50); // ������
			DrawBack(x, y); // �ٽ� ����
			DrawBack(x, y-1); // �ٽ� ����
			DrawBack(x+1, y-1); // �ٽ� ����
			DrawBack(x+1, y); // �ٽ� ����
			DrawBack(x+1, y+1); // �ٽ� ����
			DrawBack(x, y+1); // �ٽ� ����
			DrawBack(x-1, y+1); // �ٽ� ����
			DrawBack(x-1, y); // �ٽ� ����
			DrawBack(x-1, y-1); // �ٽ� ����
		}
	}
//********************************************************************************************
	// DrawScan : �ش� ��ǥ�� ��ĵ�� �׸���.
	void DrawScan(int x, int y)
	{
		Go(y*2, x);
		Color(11);
		cout << "��";
	}
//********************************************************************************************
	// DrawBack : ��ĵ�� ����� �ٽ� ��,���� �׸���.
	void DrawBack(int x, int y)
	{
		Go(y*2, x);
		if(maze[x][y] == WALL) // �ش� ��ǥ�� ���̾�����
		{
			Color(2);
			cout << "��"; // �ٽ� ���� �׸���.
		}
		else if(maze[x][y] == ROAD) // �ش� ��ǥ�� ���̾�����
		{
			if(IsNoWay(x, y)) // ���ٸ� ���̸�
			{
				maze[x][y] = WALL; // ������ ����
				map[x][y] = WALL; // ������ ����
			}
			else // ���ٸ� ���� �ƴϸ�
				cout << "��"; // �ٽ� ���� �׸���.
		}
		else if(maze[x][y] == START) // �ش� ��ǥ�� �������̾�����
		{
			Color(14);
			cout << "��"; // �ٽ� �������� �׸���.
		}
		else if(maze[x][y] == END) // �ش� ��ǥ�� �������̾�����
		{
			Color(12);
			cout << "��"; // �ٽ� �������� �׸���.
		}
	}
//********************************************************************************************
	// Go : �ܼ�â���� �ش� ��ǥ�� �̵��Ѵ�. (WinAPI)
	void Go(int x, int y) 
	{
		COORD pos = {x, y};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	}
//********************************************************************************************
	// Color : �ܼ�â�� ���� ���� �����Ѵ�. (WinAPI)/
	void Color(unsigned short color) 
	{
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hCon,color);
	}
};
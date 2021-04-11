#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include<random>
#include<stdlib.h>
#include<time.h>

#include <windows.h>
#include <io.h>
#include <wrl.h>
#include <array>
#include <direct.h>
#include "shlobj.h" 

using namespace std;

namespace FileHelper
{
	/*	Example:
		vector<string> paths;
		const string FilePath = "C:\\Users\\yivanli\\Desktop\\DX12Lab\\DX12Lab\\Textures";
		FileHelper::GetFiles(FilePath, paths);
	*/
	static void GetFiles(string path, vector<string>& files)
	{
		//文件句柄  
		long long hFile = 0;
		//文件信息  
		struct _finddata_t fileinfo;
		string p;
		if ((hFile = _findfirst(p.assign(path).c_str(), &fileinfo)) != -1)
		{
			do
			{
				//如果是目录,迭代之  
				//如果不是,加入列表  
				if ((fileinfo.attrib & _A_SUBDIR))
				{
					if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
						GetFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
				else
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}

	/*	Example:
		string path;
		FileHelper::GetProjectPath(path);
		假设在E盘的工程，那么这个值会为：
		"E:\\DX12\\DX12Lab\\DX12Lab"
	*/
	static void GetProjectPath(string& OutPath)
	{
		char* buffer;
		//也可以将buffer作为输出参数
		if ((buffer = _getcwd(NULL, 0)) != NULL)
		{
			OutPath = buffer;
		}
	}

	/*	CSIDL_BITBUCKET 回收站
		CSIDL_CONTROLS 控制面板
		CSIDL_DESKTOP Windows桌面desktop;
		CSIDL_DESKTOPDIRECTORY desktop的目录；
		CSIDL_DRIVES 我的电脑
		CSIDL_FONTS 字体目录
		CSIDL_NETHOOD 网上邻居
		CSIDL_NETWORK 网上邻居virtual folder
		CSIDL_PERSONAL 我的文档
		CSIDL_PRINTERS 打印机
		CSIDL_PROGRAMS 程序组
		CSIDL_RECENT 最近打开文档
		CSIDL_SENDTO 发送到菜单项
		CSIDL_STARTMENU 快启菜单
		CSIDL_STARTUP 启动目录
		CSIDL_TEMPLATES 临时文档
	*/

	static string GetDesktopPath()
	{
		LPITEMIDLIST pidl;
		LPMALLOC pShellMalloc;
		char szDir[1024];
		if (SUCCEEDED(SHGetMalloc(&pShellMalloc)))
		{
			if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl))) {
				// 如果成功返回true  
				SHGetPathFromIDListA(pidl, szDir);
				pShellMalloc->Free(pidl);
			}
			pShellMalloc->Release();
		}

		string Ret = string(szDir);
		//Ret.replace(Ret.find_first_of(""), filePath.length(), "")

		return Ret;
	}
}

enum class EDungeonType : char
{
	VE_None		= 0,
	VE_Wall		= 1,
	VE_Room		= 2,
	VE_RoomWall	= 3,
	VE_PassWay	= 4,
	VE_Door		= 5,
};

class FColor
{
public:

	FColor() :R(0), G(0), B(0) {}
	FColor(int r, int g, int b) :R(r), G(g), B(b) {}
	FColor(int val) :R(val), G(val), B(val) {}
	~FColor() {}
	int R, G, B;
};

class FVector2D
{
public:
	FVector2D(float x, float y) : X(x), Y(y) {}
	FVector2D():X(0.0f), Y(0.0f) {}
	~FVector2D() {}
	FVector2D operator+ (const FVector2D& Ref) { FVector2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret;}
	FVector2D operator- (const FVector2D& Ref) { FVector2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret;}
	FVector2D operator+ (const FVector2D& Ref)const { FVector2D ret; ret.X = this->X + Ref.X; ret.Y = this->Y + Ref.Y; return ret; }
	FVector2D operator- (const FVector2D& Ref)const { FVector2D ret; ret.X = this->X - Ref.X; ret.Y = this->Y - Ref.Y; return ret; }
	FVector2D operator* (const FVector2D& Ref){ FVector2D ret; ret.X = this->X * Ref.X; ret.Y = this->Y * Ref.Y; return ret;}
	FVector2D operator* (const float& fval){ FVector2D ret; ret.X = this->X * fval; ret.Y = this->Y * fval; return ret;}
	FVector2D operator/ (const FVector2D& Ref){ FVector2D ret; ret.X = this->X / Ref.X; ret.Y = this->Y / Ref.Y; return ret;}
	FVector2D operator/ (const float& fval){ FVector2D ret; ret.X = this->X / fval; ret.Y = this->Y / fval; return ret;}
	void operator= (const FVector2D& Ref){ this->X = Ref.X; this->Y = Ref.Y;}
	bool operator== (const FVector2D& Ref)const { return this->X == Ref.X && this->Y == Ref.Y; }
	bool operator!= (const FVector2D& Ref)const { return this->X != Ref.X || this->Y != Ref.Y; }
	void Print(){cout << "X: " << X << "  " << "Y: " << Y << endl; }
	float X, Y;
};

class FImage
{
public:

	//创建默认是黑色填充
	FImage(int X, int Y, string ImageName) :
		SizeX(X),
		SizeY(Y),
		ImageName(ImageName)
	{
		for (int i = 0; i < SizeX; i++)
		{
			vector<FColor> row(SizeY);
			Imagedata.push_back(row);
		}
	}

	bool SaveImageToDesk()
	{
		const string& Path = FileHelper::GetDesktopPath();

		cout << endl << "begin save image to desk operation" << endl;

		bool bCreateImage = false;

		ofstream fout(Path + "/" + ImageName + ".ppm");
		fout << "P3\n" << SizeX << " " << SizeY << "\n255\n";

		for (int y = 0; y < SizeY; y++)
		{
			for (int x = 0; x < SizeX; x++)
			{
				FColor& color = Imagedata[x][y];
				fout << color.R << " " << color.G << " " << color.B << "\n";
			}
		}
		fout.close();

		cout << endl << "Save image successfully" << endl;

		return bCreateImage;
	}

	void ClearImage(FColor& ClearColor)
	{
		cout << endl << "Begin clear image operation" << endl;

		int nx = SizeX;
		int ny = SizeY;

		for (int i = 0; i < nx; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				Imagedata[i][j] = ClearColor;
			}
		}
	}

	void SetPixleColor(const FColor& newData, const FVector2D& PixleLocation)
	{
		if (PixleLocation.X < SizeX && PixleLocation.Y < SizeY)
			Imagedata[PixleLocation.X][PixleLocation.Y] = newData;
	}
	void SetPixleColor(const int& newData, const FVector2D& PixleLocation)
	{
		if (PixleLocation.X < SizeX && PixleLocation.Y < SizeY)
			Imagedata[PixleLocation.X][PixleLocation.Y] = FColor(newData);
	}

	int SizeX;
	int SizeY;
	string ImageName;
	string ImagePath;
	vector<vector<FColor>> Imagedata;
};

struct FDungeon
{
	FDungeon() : DungeonSizeX(0), DungeonSizeY(0){}
	FDungeon(const int& x, const int& y, const EDungeonType& inType) : DungeonSizeX(x), DungeonSizeY(y)
	{
		//初始化地下城，让地下城全部是墙
		for (int y = 0; y < DungeonSizeY; y++)
		{
			vector<EDungeonType> yarray;
			Data.push_back(std::move(yarray));
			for (int x = 0; x < DungeonSizeX; x++)
			{
				Data[y].push_back(inType);
			}
		}
	}

	EDungeonType Get(const FVector2D& Loc)
	{
		if(Loc.X >= 0 && Loc.X < DungeonSizeX && Loc.Y >= 0 && Loc.Y <= DungeonSizeY)
			return Data[Loc.X][Loc.Y];
		else
			return EDungeonType::VE_None;
	}

	vector<vector<EDungeonType>> Data;
	//只能是奇数
	int DungeonSizeX;
	int DungeonSizeY;
};

struct FRoom
{

	FRoom(){}
	FRoom(const FVector2D& loc, const FVector2D& xyEx) : Location(loc), XYExtent(xyEx)
	{
		min = loc - xyEx;
		max = loc + xyEx;
	}

	FVector2D Location;
	FVector2D XYExtent;

	FVector2D min, max;
};

float RandInRange(const int& min, const int& max)
{
	return (rand() % (max - min + 1)) + min;
}

bool InRange(const float& val, const float& min, const float& max, bool includeBoarder = false)
{
	if (includeBoarder){ if (val >= min && val <= max) return true; else return false; }
	else { if (val > min && val < max) return true; else return false;}
}

FVector2D GetRandDirection()
{
	const float& XAxis = RandInRange(-1.5f, 1.5f);
	const float& YAxis = RandInRange(-1.5f, 1.5f);

	FVector2D ret;

	if (InRange(XAxis, -1.5f, -0.5f)) ret.X = -1;
	if (InRange(XAxis, -0.5f, 0.5f)) ret.X = 0;
	if (InRange(XAxis, 0.5f, 1.5f)) ret.X = 1;

	if (InRange(YAxis, -1.5f, -0.5f)) ret.Y = -1;
	if (InRange(YAxis, -0.5f, 0.5f)) ret.Y = 0;
	if (InRange(YAxis, 0.5f, 1.5f)) ret.Y = 1;

	return ret;
}

void CrossFindPointOrigPointMap(const FVector2D& PointVal, const vector<vector<FVector2D>>* OrigArrayMap, vector<FVector2D>* OutPoints)
{
	//PointVal是顶点在整个网格里的位置
	int SizeX = OrigArrayMap[0].size();
	int SizeY = OrigArrayMap->size();

	//找到PointVal在OrigArrayMap里的位置
	FVector2D PointInOrigArrayMap;
	for (int y = 0; y < SizeY; y++)
	{
		bool bBreak = false;
		for (int x = 0; x < SizeX; x++)
		{
			if ((*OrigArrayMap)[x][y] == PointVal)
			{
				PointInOrigArrayMap.X = x;
				PointInOrigArrayMap.Y = y;
				bBreak = true;
				break;
			}
		}
		if (bBreak) break;
	}

	if(PointInOrigArrayMap.X - 1 >= 0)
		OutPoints->push_back((*OrigArrayMap)[PointInOrigArrayMap.X - 1][PointInOrigArrayMap.Y]);
	if (PointInOrigArrayMap.X + 1 < SizeX)
		OutPoints->push_back((*OrigArrayMap)[PointInOrigArrayMap.X + 1][PointInOrigArrayMap.Y]);
	if (PointInOrigArrayMap.Y - 1 >= 0)
		OutPoints->push_back((*OrigArrayMap)[PointInOrigArrayMap.X][PointInOrigArrayMap.Y - 1]);
	if (PointInOrigArrayMap.Y + 1 < SizeY)
		OutPoints->push_back((*OrigArrayMap)[PointInOrigArrayMap.X][PointInOrigArrayMap.Y + 1]);
}

//2D AABB box 相交测试，如果相交返回true，如果没有返回false
bool IsAABB2DIntersect(const FVector2D& A_Min, const FVector2D& A_Max, const FVector2D& B_Min, const FVector2D& B_Max)
{
	return A_Min.X < B_Max.X&& A_Max.X > B_Min.X && A_Min.Y < B_Max.Y&& A_Max.Y > B_Min.Y;
}

int main()
{
	//初始化随机数种子
	//srand((int)time(NULL));
	srand(0);

	//只能是奇数
	const int SizeX = 45;
	const int SizeY = 45;
	FImage* OutputImage = new FImage(SizeX, SizeY, "OutputImage");
	
	FDungeon* Dungeon = new FDungeon(SizeX, SizeY, EDungeonType::VE_Wall);

	//创建网格
	vector<FVector2D>OrigPoint;
	for (int y = 1; y < SizeY; y+=2)
	{
		for (int x = 1; x < SizeX; x+=2)
		{
			if (x % 2 == 0)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_Wall;
				OrigPoint.push_back(std::move(FVector2D(x,y)));
			}
			else if (x % 2 == 1)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_PassWay;
				OrigPoint.push_back(std::move(FVector2D(x, y)));
			}
			else if (y % 2 == 0)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_Wall;
				OrigPoint.push_back(std::move(FVector2D(x, y)));
			}
			else if (y % 2 == 1)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_PassWay;
				OrigPoint.push_back(std::move(FVector2D(x, y)));
			}
		}
	}

	//把起始格子的空白格子的位置放到二维数组里
	vector<vector<FVector2D>>* OrigPointMap = new vector<vector<FVector2D>>();
	float OX = OrigPoint[0].X;
	float OY = OrigPoint[0].Y;
	int YIndex = 0;
	for (const FVector2D& p : OrigPoint)
	{
		if (p.X == OX)
		{
			vector<FVector2D> yarray;
			OrigPointMap->push_back(std::move(yarray));
		}

		if (p.Y == OY)
		{
			(*OrigPointMap)[YIndex].push_back(p);
		}
		else
		{
			YIndex++;
			OY = p.Y;
			(*OrigPointMap)[YIndex].push_back(p);
		}
	}

	//创建道路
	//找到一个起始点
	const FVector2D& StartPoint = OrigPoint[RandInRange(0, OrigPoint.size() - 1)];

	vector<FVector2D>* UsedPointList = new vector<FVector2D>();
	vector<FVector2D>* ToBeUsePointList = new vector<FVector2D>();
	FVector2D CurrentPoint = StartPoint;
	vector<FVector2D>* LastPointArray = new vector<FVector2D>();

	bool bFinish = false;
	while (true)
	{
		UsedPointList->push_back(CurrentPoint);
		ToBeUsePointList->clear();
		CrossFindPointOrigPointMap(CurrentPoint, OrigPointMap, ToBeUsePointList);

		vector<FVector2D>CanBeUsedPointList;
		for (const FVector2D& ToBeP : *ToBeUsePointList)
		{
			bool bCanUse = true;
			for (const FVector2D& UsedP : *UsedPointList)
			{
				if (UsedP == ToBeP)
					bCanUse = false;
			}

			if (bCanUse == true)
				CanBeUsedPointList.push_back(ToBeP);
		}

		if (CanBeUsedPointList.size() == 0)
		{
			if (LastPointArray->size() != 0)
			{
				CurrentPoint = (*LastPointArray)[LastPointArray->size() - 1];
				LastPointArray->pop_back();
			}
			else
			{
				bFinish = true;
			}
		}
		else
		{
			int CanBeUsedPointNumber = CanBeUsedPointList.size();
			FVector2D SelectPoint = CanBeUsedPointList[RandInRange(0, CanBeUsedPointList.size() - 1)];

			FVector2D DestWallLoc = (CurrentPoint + SelectPoint) / FVector2D(2, 2);

			Dungeon->Data[DestWallLoc.X][DestWallLoc.Y] = EDungeonType::VE_PassWay;

			LastPointArray->push_back(CurrentPoint);
			CurrentPoint = SelectPoint;
		}

		CurrentPoint.Print();

		if (bFinish == true)
			break;
	}
	
	//创建房间
	const int CreateRoomAttempNum = 100;
	const int RoomMaxSizeXExtent = 5;
	const int RoomMaxSizeYExtent = 5;
	vector<FRoom*>* AllRoom = new vector<FRoom*>();
	for (int attemp = 0; attemp < CreateRoomAttempNum; attemp++)
	{
		int RoomXExtent = RandInRange(2, RoomMaxSizeXExtent);
		int RoomYExtent = RandInRange(2, RoomMaxSizeYExtent);

		const FVector2D& RoomLocation = FVector2D((int)RandInRange(0, SizeX - 1), (int)RandInRange(0, SizeY - 1));
		if (
			RoomLocation.X - RoomXExtent > 0 &&
			RoomLocation.X + RoomXExtent < SizeX &&
			RoomLocation.Y - RoomYExtent > 0 &&
			RoomLocation.Y + RoomYExtent < SizeY)
		{
			FRoom* newRoom = new FRoom(RoomLocation, FVector2D(RoomXExtent, RoomYExtent));
			bool bCanAdd = true;
			for (const FRoom* room : *AllRoom)
			{
				const FVector2D& Room_A_Min = room->min;
				const FVector2D& Room_A_Max = room->max;
				const FVector2D& Room_B_Min = newRoom->min;
				const FVector2D& Room_B_Max = newRoom->max;
				if (IsAABB2DIntersect(Room_A_Min, Room_A_Max, Room_B_Min, Room_B_Max))
				{
					bCanAdd = false;
				}
			}
			if (bCanAdd == true)
				AllRoom->push_back(newRoom);
		}
	}

	//把每个房间的墙的位置保存下来，供以后开门用
	vector<vector<FVector2D>*>* AllRoomWallLocation = new vector<vector<FVector2D>*>();
	for (const FRoom* room : *AllRoom)
	{
		vector<FVector2D>* OneRoomWall = new vector<FVector2D>();
		AllRoomWallLocation->push_back(OneRoomWall);
		for (int y = room->min.Y; y <= room->max.Y; y++)
		{
			for (int x = room->min.X; x <= room->max.X; x++)
			{
				if (x == room->min.X || x == room->max.X || y == room->min.Y || y == room->max.Y)
				{
					Dungeon->Data[x][y] = EDungeonType::VE_RoomWall;
					OneRoomWall->push_back(FVector2D(x, y));
				}
				else
					Dungeon->Data[x][y] = EDungeonType::VE_Room;
			}
		}
	}

	//开门
	//房间的上下左右wall
	vector<FVector2D>* TopWall = new vector<FVector2D>();
	vector<FVector2D>* DownWall = new vector<FVector2D>();
	vector<FVector2D>* LeftWall = new vector<FVector2D>();
	vector<FVector2D>* RightWall = new vector<FVector2D>();
	for (const vector<FVector2D>* SingleRoomWall : *AllRoomWallLocation)
	{
		TopWall->clear();
		DownWall->clear();
		LeftWall->clear();
		RightWall->clear();
		for (const FVector2D& roomWall : *SingleRoomWall)
		{
			const FVector2D& uLoc = roomWall + FVector2D(0, 1);
			const FVector2D& dLoc = roomWall + FVector2D(0, -1);
			const FVector2D& lLoc = roomWall + FVector2D(-1, 0);
			const FVector2D& rLoc = roomWall + FVector2D(1, 0);

			//top
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(dLoc) == EDungeonType::VE_Room && 
				Dungeon->Get(lLoc) == EDungeonType::VE_RoomWall&&
				Dungeon->Get(rLoc) == EDungeonType::VE_RoomWall
			)
			{
				TopWall->push_back(roomWall);
			}
			//down
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_Room &&
				Dungeon->Get(dLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(lLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(rLoc) == EDungeonType::VE_RoomWall
				)
			{
				DownWall->push_back(roomWall);
			}
			//left
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(dLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(lLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(rLoc) == EDungeonType::VE_Room
				)
			{
				LeftWall->push_back(roomWall);
			}
			//Right
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(dLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(lLoc) == EDungeonType::VE_Room &&
				Dungeon->Get(rLoc) == EDungeonType::VE_PassWay
				)
			{
				RightWall->push_back(roomWall);
			}
		}
		if (TopWall->size() > 0)
		{
			const int& id = (int)RandInRange(0, TopWall->size() - 1);
			const FVector2D& DoorLoc = (*TopWall)[id];
			Dungeon->Data[DoorLoc.X][DoorLoc.Y] = EDungeonType::VE_Door;
		}
		if (DownWall->size() > 0)
		{
			const int& id = (int)RandInRange(0, DownWall->size() - 1);
			const FVector2D& DoorLoc = (*DownWall)[id];
			Dungeon->Data[DoorLoc.X][DoorLoc.Y] = EDungeonType::VE_Door;
		}
		if (LeftWall->size() > 0)
		{
			const int& id = (int)RandInRange(0, LeftWall->size() - 1);
			const FVector2D& DoorLoc = (*LeftWall)[id];
			Dungeon->Data[DoorLoc.X][DoorLoc.Y] = EDungeonType::VE_Door;
		}
		if (RightWall->size() > 0)
		{
			const int& id = (int)RandInRange(0, RightWall->size() - 1);
			const FVector2D& DoorLoc = (*RightWall)[id];
			Dungeon->Data[DoorLoc.X][DoorLoc.Y] = EDungeonType::VE_Door;
		}
	}
	

	//渲染输出的图片
	for (int y = 0; y < SizeY; y++)
	{
		for (int x = 0; x < SizeX; x++)
		{
			if (Dungeon->Data[x][y] == EDungeonType::VE_Wall)
				OutputImage->SetPixleColor(0, FVector2D(x, y));

			if (Dungeon->Data[x][y] == EDungeonType::VE_PassWay)
				OutputImage->SetPixleColor(255, FVector2D(x, y));

			if (Dungeon->Data[x][y] == EDungeonType::VE_Room)
				OutputImage->SetPixleColor(FColor(0, 0, 255), FVector2D(x, y));

			if (Dungeon->Data[x][y] == EDungeonType::VE_RoomWall)
				OutputImage->SetPixleColor(FColor(0, 255, 0), FVector2D(x, y));

			if (Dungeon->Data[x][y] == EDungeonType::VE_Door)
				OutputImage->SetPixleColor(FColor(255, 0, 0), FVector2D(x, y));
		}
	}

	OutputImage->SaveImageToDesk();

	return 0;
}
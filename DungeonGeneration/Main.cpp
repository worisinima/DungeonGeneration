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
	VE_None = 0,
	VE_Wall,
	VE_RoomWall,
	VE_Room,
	VE_PassWay,
	VE_Door,
};
enum class ESlotMovementState : char
{
	MS_Forward,
	MS_Backward
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
	FDungeon() : DungeonSizeX(0), DungeonSizeY(0), OutputImage(nullptr){}
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

		OutputImage = new FImage(DungeonSizeX, DungeonSizeY, "OutputImage");
	}

	EDungeonType Get(const FVector2D& Loc) const
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

	FImage* OutputImage;
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

void LookingForPotentialPoints(const FVector2D& CurrentPoint, const FDungeon* Dungon, vector<FVector2D>& OutPotentialPoints)
{
	//PointVal是顶点在整个网格里的位置
	const int& SizeX = Dungon->DungeonSizeX;
	const int& SizeY = Dungon->DungeonSizeY;
	const int x = CurrentPoint.X;
	const int y = CurrentPoint.Y;

	if (y + 2 < SizeY && Dungon->Get(FVector2D(x, y + 2)) == EDungeonType::VE_PassWay)
		OutPotentialPoints.push_back(FVector2D(x, y + 2));

	if (y - 2 > 0 && Dungon->Get(FVector2D(x, y - 2)) == EDungeonType::VE_PassWay)
		OutPotentialPoints.push_back(FVector2D(x, y - 2));

	if (x - 2 > 0 && Dungon->Get(FVector2D(x - 2, y)) == EDungeonType::VE_PassWay)
		OutPotentialPoints.push_back(FVector2D(x - 2, y));

	if (x + 2 < SizeX && Dungon->Get(FVector2D(x + 2, y)) == EDungeonType::VE_PassWay)
		OutPotentialPoints.push_back(FVector2D(x + 2, y));

}

bool HaveFindDoorAround(const FVector2D& CurrentPoint, const FDungeon* Dungon)
{
	//PointVal是顶点在整个网格里的位置
	const int& SizeX = Dungon->DungeonSizeX;
	const int& SizeY = Dungon->DungeonSizeY;
	const int x = CurrentPoint.X;
	const int y = CurrentPoint.Y;

	if (y + 1 < SizeY && Dungon->Get(FVector2D(x, y + 1)) == EDungeonType::VE_Door)
		return true;

	if (y - 1 > 0 && Dungon->Get(FVector2D(x, y - 1)) == EDungeonType::VE_Door)
		return true;

	if (x - 1 > 0 && Dungon->Get(FVector2D(x - 1, y)) == EDungeonType::VE_Door)
		return true;

	if (x + 1 < SizeX && Dungon->Get(FVector2D(x + 1, y)) == EDungeonType::VE_Door)
		return true;

	return false;
}

//2D AABB box 相交测试，如果相交返回true，如果没有返回false
bool IsAABB2DIntersect(const FVector2D& A_Min, const FVector2D& A_Max, const FVector2D& B_Min, const FVector2D& B_Max)
{
	return A_Min.X < B_Max.X&& A_Max.X > B_Min.X && A_Min.Y < B_Max.Y&& A_Max.Y > B_Min.Y;
}

void RenderDungeon(FDungeon* Dungeon)
{
	const int SizeY = Dungeon->DungeonSizeX;
	const int SizeX = Dungeon->DungeonSizeX;

	//渲染输出的图片
	for (int y = 0; y < SizeY; y++)
	{
		for (int x = 0; x < SizeX; x++)
		{
			switch (Dungeon->Data[x][y])
			{
			case EDungeonType::VE_None:
				Dungeon->OutputImage->SetPixleColor(255, FVector2D(x, y));
				break;
			case EDungeonType::VE_Wall:
				Dungeon->OutputImage->SetPixleColor(0, FVector2D(x, y));
				break;
			case EDungeonType::VE_RoomWall:
				Dungeon->OutputImage->SetPixleColor(0, FVector2D(x, y));
				break;
			case EDungeonType::VE_Room:
				Dungeon->OutputImage->SetPixleColor(FColor(0, 0, 255), FVector2D(x, y));
				break;
			case EDungeonType::VE_Door:
				Dungeon->OutputImage->SetPixleColor(FColor(255, 0, 0), FVector2D(x, y));
				break;
			case EDungeonType::VE_PassWay:
				Dungeon->OutputImage->SetPixleColor(255, FVector2D(x, y));
				break;
			default:
				Dungeon->OutputImage->SetPixleColor(255, FVector2D(x, y));
				break;
			}
		}
	}

	Dungeon->OutputImage->SaveImageToDesk();
}

int main()
{
	//初始化随机数种子
	//srand((int)time(NULL));
	srand(0);

	//只能是奇数
	const int SizeY = 81;
	const int SizeX = 81;
	FDungeon* Dungeon = new FDungeon(SizeX, SizeY, EDungeonType::VE_PassWay);
	
	//生成最边缘的墙体
	for (int y = 0; y < SizeY; y++)
	{
		for (int x = 0; x < SizeX; x++)
		{
			if(y == 0 || y == SizeY - 1 || x == 0 || x == SizeX - 1)
				Dungeon->Data[x][y] = EDungeonType::VE_Wall;
		}
	}

	//创建网格
	vector<FVector2D>OrigPoint;
	for (int y = 0; y < SizeY; y ++)
	{
		for (int x = 0; x < SizeX; x ++)
		{
			if (x % 2 == 0 || y % 2 == 0)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_Wall;
				OrigPoint.push_back(std::move(FVector2D(x, y)));
			}
			else if(x % 2 == 1 || y % 2 == 1)
			{
				Dungeon->Data[x][y] = EDungeonType::VE_PassWay;
				OrigPoint.push_back(std::move(FVector2D(x, y)));
			}
		}
	}

	//创建房间
	const int CreateRoomAttempNum = 1000;
	const int RoomMaxSizeXExtent = 6;
	const int RoomMaxSizeYExtent = 6;
	vector<FRoom*>* AllRoom = new vector<FRoom*>();
	for (int attemp = 0; attemp < CreateRoomAttempNum; attemp++)
	{
		int RoomXExtent = RandInRange(4, RoomMaxSizeXExtent);
		int RoomYExtent = RandInRange(4, RoomMaxSizeYExtent);
		RoomXExtent = RoomXExtent % 2 == 1 ? RoomXExtent : RoomXExtent - 1;
		RoomYExtent = RoomYExtent % 2 == 1 ? RoomYExtent : RoomYExtent - 1;

		const FVector2D& RoomLocation = FVector2D((int)RandInRange(0, SizeX - 1), (int)RandInRange(0, SizeY - 1));
		if (
			RoomLocation.X - RoomXExtent > 0 &&
			RoomLocation.X + RoomXExtent < SizeX - 1 &&
			RoomLocation.Y - RoomYExtent > 0 &&
			RoomLocation.Y + RoomYExtent < SizeY - 1 &&
			(int)RoomLocation.X % 2 == 1 &&
			(int)RoomLocation.Y % 2 == 1
		)
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
	vector<vector<FVector2D>*>* WallListOfEachRoom = new vector<vector<FVector2D>*>();
	for (const FRoom* room : *AllRoom)
	{
		vector<FVector2D>* RoomWall = new vector<FVector2D>();
		WallListOfEachRoom->push_back(RoomWall);
		for (int y = room->min.Y; y <= room->max.Y; y++)
		{
			for (int x = room->min.X; x <= room->max.X; x++)
			{
				if (x == room->min.X || x == room->max.X || y == room->min.Y || y == room->max.Y)
				{
					Dungeon->Data[x][y] = EDungeonType::VE_RoomWall;
					RoomWall->push_back(FVector2D(x, y));
				}
				else
					Dungeon->Data[x][y] = EDungeonType::VE_Room;
			}
		}
	}

	//寻找一个寻路的迭代起始点
	vector<FVector2D>* AllPasswayLoc = new vector<FVector2D>();
	for (int y = 0; y < SizeY; y++)
	{
		for (int x = 0; x < SizeX; x++)
		{
			if(Dungeon->Data[x][y] == EDungeonType::VE_PassWay)
				AllPasswayLoc->push_back(FVector2D(x, y));
		}
	}
	const FVector2D& StartPoint = (*AllPasswayLoc)[RandInRange(0, AllPasswayLoc->size() - 1)];
	FVector2D CurrentPoint = StartPoint;
	FVector2D LastPoint = CurrentPoint;

	vector<FVector2D>* UsedPointList = new vector<FVector2D>();
	vector<FVector2D>* LastPointList = new vector<FVector2D>();

	ESlotMovementState CurrentMovementState = ESlotMovementState::MS_Forward;
	ESlotMovementState LastMovementState = ESlotMovementState::MS_Forward;
	vector<FVector2D>* CurrentBackRoadPtr = nullptr;
	vector<vector<FVector2D>*>* DeadEndRoadList = new vector<vector<FVector2D>*>();

	bool bFinish = false;
	while (true)
	{
		//寻找周围潜在的移动点
		vector<FVector2D> ProtentialPointList;
		ProtentialPointList.clear();
		UsedPointList->push_back(CurrentPoint);
		LookingForPotentialPoints(CurrentPoint, Dungeon, ProtentialPointList);

		//在周围的潜在移动点中寻找没有被走过的点
		vector<FVector2D>CanBeUsedPointList;
		for (const FVector2D& ToBeP : ProtentialPointList)
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

		//如果当前点周围没有可以使用的移动点说明移动到了死胡同，那么就往回退
		if (CanBeUsedPointList.size() == 0)
		{
			CurrentMovementState = ESlotMovementState::MS_Backward;

			if (LastPointList->size() != 0)
			{
				LastPoint = CurrentPoint;
				CurrentPoint = (*LastPointList)[LastPointList->size() - 1];
				LastPointList->pop_back();
			}
			else
			{
				bFinish = true;
			}
		}
		//如果当前点周围有可以使用的移动点，那么就随机选取一个点移动过去
		else
		{
			CurrentMovementState = ESlotMovementState::MS_Forward;

			int CanBeUsedPointNumber = CanBeUsedPointList.size();
			FVector2D SelectPoint = CanBeUsedPointList[RandInRange(0, CanBeUsedPointList.size() - 1)];

			FVector2D DestWallLoc = (CurrentPoint + SelectPoint) / FVector2D(2, 2);

			Dungeon->Data[DestWallLoc.X][DestWallLoc.Y] = EDungeonType::VE_PassWay;

			LastPointList->push_back(CurrentPoint);
			LastPoint = CurrentPoint;
			CurrentPoint = SelectPoint;
		}

		if (CurrentMovementState == ESlotMovementState::MS_Forward && LastMovementState == ESlotMovementState::MS_Forward)
		{
			//什么事情也不会发生
		}
		else if (CurrentMovementState == ESlotMovementState::MS_Backward && LastMovementState == ESlotMovementState::MS_Forward)
		{
			//第一次倒退，开始创建倒退列表
			vector<FVector2D>* BackRoad = new vector<FVector2D>();
			CurrentBackRoadPtr = BackRoad;
			CurrentBackRoadPtr->push_back(LastPoint);
			
			FVector2D MoveDir = (CurrentPoint - LastPoint) / 2.0f;
			FVector2D RoadMidLoc = LastPoint + MoveDir;
			CurrentBackRoadPtr->push_back(RoadMidLoc);

			CurrentBackRoadPtr->push_back(CurrentPoint);

			DeadEndRoadList->push_back(CurrentBackRoadPtr);
		}
		else if (CurrentMovementState == ESlotMovementState::MS_Backward && LastMovementState == ESlotMovementState::MS_Backward)
		{
			//在倒退途中
			if (CurrentBackRoadPtr != nullptr)
			{
				FVector2D MoveDir = (CurrentPoint - LastPoint) / 2.0f;
				FVector2D RoadMidLoc = LastPoint + MoveDir;
				CurrentBackRoadPtr->push_back(RoadMidLoc);

				CurrentBackRoadPtr->push_back(CurrentPoint);
			}
		}
		else if (CurrentMovementState == ESlotMovementState::MS_Forward && LastMovementState == ESlotMovementState::MS_Backward)
		{
			//终止倒退
			CurrentBackRoadPtr->push_back(LastPoint);

			FVector2D MoveDir = (CurrentPoint - LastPoint) / 2.0f;
			FVector2D RoadMidLoc = LastPoint + MoveDir;
			CurrentBackRoadPtr->push_back(RoadMidLoc);

			CurrentBackRoadPtr = nullptr;
		}

		//打印当前点的位置
		CurrentPoint.Print();

		LastMovementState = CurrentMovementState;

		//完成迭代跳出循环
		if (bFinish == true)
			break;
	}

	vector<FVector2D>* TopWall = new vector<FVector2D>();
	vector<FVector2D>* DownWall = new vector<FVector2D>();
	vector<FVector2D>* LeftWall = new vector<FVector2D>();
	vector<FVector2D>* RightWall = new vector<FVector2D>();
	for (const vector<FVector2D>* Roomwall : *WallListOfEachRoom)
	{
		TopWall->clear();
		DownWall->clear();
		LeftWall->clear();
		RightWall->clear();

		for (const FVector2D& roomWallLoc : *Roomwall)
		{
			const FVector2D& uLoc = roomWallLoc + FVector2D(0, 1);
			const FVector2D& dLoc = roomWallLoc + FVector2D(0, -1);
			const FVector2D& lLoc = roomWallLoc + FVector2D(-1, 0);
			const FVector2D& rLoc = roomWallLoc + FVector2D(1, 0);

			//top
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(dLoc) == EDungeonType::VE_Room &&
				Dungeon->Get(lLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(rLoc) == EDungeonType::VE_RoomWall
				)
			{
				TopWall->push_back(roomWallLoc);
			}
			//down
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_Room &&
				Dungeon->Get(dLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(lLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(rLoc) == EDungeonType::VE_RoomWall
				)
			{
				DownWall->push_back(roomWallLoc);
			}
			//left
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(dLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(lLoc) == EDungeonType::VE_PassWay &&
				Dungeon->Get(rLoc) == EDungeonType::VE_Room
				)
			{
				LeftWall->push_back(roomWallLoc);
			}
			//Right
			if (
				Dungeon->Get(uLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(dLoc) == EDungeonType::VE_RoomWall &&
				Dungeon->Get(lLoc) == EDungeonType::VE_Room &&
				Dungeon->Get(rLoc) == EDungeonType::VE_PassWay
				)
			{
				RightWall->push_back(roomWallLoc);
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

	//修剪支路死路
	const int RoadCutDepth = 10;
	for (const vector<FVector2D>* road : *DeadEndRoadList)
	{
		for (int depth = 0; depth < RoadCutDepth; depth++)
		{
			if (depth < road->size())
			{
				const vector<FVector2D>& roaddata = *road;

				//查看一下周围是不是有门，如果有的话就不能封路了
				if (HaveFindDoorAround(roaddata[depth], Dungeon) == true)
					break;

				Dungeon->Data[roaddata[depth].X][roaddata[depth].Y] = EDungeonType::VE_Wall;
			}
		}
	}

	//渲染整个地下城，渲染结果会被存到桌面
	RenderDungeon(Dungeon);

	return 0;
}
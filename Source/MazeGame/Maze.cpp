// Fill out your copyright notice in the Description page of Project Settings.
#include "Maze.h"
#include "Math/UnrealMathUtility.h"
// Sets default values
AMaze::AMaze()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	RootComponent = Floor;
}

// Called when the game starts or when spawned
void AMaze::BeginPlay()
{
	Super::BeginPlay();
	GenerateMaze();
}

// Called every frame
void AMaze::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMaze::AddPassage(TArray<int32> A, TArray<int32> B)
{
	if ((B[0] - A[0] == 1) && (B[1] == A[1])) Passages[A[0]][A[1]][EAST] = true;
	else if ((B[0] - A[0] == -1) && (B[1] == A[1])) Passages[A[0]][A[1]][WEST] = true;
	else if ((B[1] - A[1] == 1) && (B[0] == A[0])) Passages[A[0]][A[1]][NORTH] = true;
	else if ((B[1] - A[1] == -1) && (B[0] == A[0])) Passages[A[0]][A[1]][SOUTH] = true;
}

TArray<int32> AMaze::ChooseRandomlyFrom(TArray<TArray<int32>> List)
{
	if (List.Num() == 0) return {-10, -10};
	int Index = FMath::RandRange(0, List.Num() - 1);
	return List[Index];
}

TArray<int32> AMaze::ExpandLocation(TArray<int32> Here, int32 Direction)
{
	TArray<int32> There = {0, 0};
	There[X] = Here[X] + OFFSETS[Direction][X];
	There[Y] = Here[Y] + OFFSETS[Direction][Y];
	if (Unexplored.Find(There) != INDEX_NONE) 
	{
		AddPassage(Here, There);
		return There;
	} 
	else return {-10, -10};
}

TArray<int32> AMaze::ExpandMaze()
{
	TArray<int32> Here;
	if (LastExploredLocation[0] == -10 && LastExploredLocation[1] == -10) Here = ChooseRandomlyFrom(Frontier);
	else Here = LastExploredLocation;

	int32 Direction = FMath::RandRange(0, 3);
	for (int32 i = 0; i < 4; i++) 
	{
		TArray<int32> There = ExpandLocation(Here, Direction); 
		if (!(There[0] == -10 && There[1] == -10))
		{
			Frontier.Insert(There, 0);
			Unexplored.Remove(There);
			return There;
		}
		Direction = (Direction + 1) % 4;
	}
	Done.Insert(Here, 0);
	Frontier.Remove(Here);
	return {-10, -10};
}

void AMaze::GenerateMaze()
{
	TArray<TArray<bool>> Passage;
	Passage.Init({false, false, false, false}, MAZE_SIZE);
	Passages.Init(Passage, MAZE_SIZE);

	Frontier = {{ 0, 0 }};
	LastExploredLocation = {-10, -10};

	int32 i = 0;
	for (int x = 0; x < MAZE_SIZE; x++) 
	{
		for (int y = 0; y < MAZE_SIZE; y++) 
		{
			Unexplored.Insert({ x, y }, 0);
			i++;
		}
	}
	Unexplored.Remove({0, 0});

	SpawnCorners();

	while (Unexplored.Num() > 0) 
	{
		LastExploredLocation = ExpandMaze();
	}

	SpawnWalls();
}

void AMaze::SpawnCorners()
{
	FVector SpawnLocation;
	SpawnLocation.X = 0.0;
	SpawnLocation.Y = 0.0;
	SpawnLocation.Z = Height / 2;

	FRotator SpawnRotation;
	SpawnRotation.Roll = 0.0;
	SpawnRotation.Pitch = 0.0;
	SpawnRotation.Yaw = 0.0;

	for (int i = 0; i < MAZE_SIZE + 1; i++)
	{
		for (int j = 0; j < MAZE_SIZE + 1; j++)
		{
			SpawnLocation.X = i * WallWidth * (1 + PassageWidthToWallWidthRatio) + WallWidth / 2;
			SpawnLocation.Y = j * WallWidth * (1 + PassageWidthToWallWidthRatio) + WallWidth / 2;
			GetWorld()->SpawnActor<AActor>(CornerClass, SpawnLocation, SpawnRotation);
		}
	}
}

void AMaze::SpawnWalls()
{
	FVector SpawnLocation;
	SpawnLocation.X = 0.0;
	SpawnLocation.Y = 0.0;
	SpawnLocation.Z = Height / 2;

	FRotator SpawnRotation;
	SpawnRotation.Roll = 0.0;
	SpawnRotation.Pitch = 0.0;

	for (int i = 0; i < MAZE_SIZE; i++)
	{
		SpawnRotation.Yaw = 90.0;
		if (i != 0)
		{
			SpawnLocation.X = WallWidth * (i * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
			SpawnLocation.Y = WallWidth / 2;
			GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);
		}

		SpawnLocation.X = WallWidth * (i * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
		SpawnLocation.Y = MapLength - WallWidth / 2;
		GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);

		SpawnRotation.Yaw = 0.0;
		SpawnLocation.X = WallWidth / 2;
		SpawnLocation.Y = WallWidth * (i * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
		GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);

		if (i != MAZE_SIZE - 1)
		{
			SpawnLocation.X = MapLength - WallWidth / 2;
			SpawnLocation.Y = WallWidth * (i * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
			GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);
		}
	}
	for (int x = 0; x < Passages.Num(); x++)
	{
		for (int y = 0; y < Passages.Num(); y++)
		{
			if (!(Passages[x][y][NORTH] || ((y + 1 < Passages.Num()) && Passages[x][y + 1][SOUTH])))
			{
				SpawnRotation.Yaw = 90.0;
				SpawnLocation.X = WallWidth * (x * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
				SpawnLocation.Y = (y + 1) * WallWidth * (1 + PassageWidthToWallWidthRatio) + WallWidth / 2;
				GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);
			}
			if (!(Passages[x][y][EAST] || ((x + 1 < Passages.Num()) && Passages[x + 1][y][WEST])))
			{
				if (!((x == Passages.Num() - 1) && (y == Passages.Num() - 1)))
				{
					SpawnRotation.Yaw = 0.0;
					SpawnLocation.X = (x + 1) * WallWidth * (1 + PassageWidthToWallWidthRatio) + WallWidth / 2;
					SpawnLocation.Y = WallWidth * (y * (1 + PassageWidthToWallWidthRatio) + PassageWidthToWallWidthRatio / 2 + 1);
					GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, SpawnRotation);
				}
			}
		}
	}
}

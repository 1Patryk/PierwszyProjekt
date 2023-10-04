// Fill out your copyright notice in the Description page of Project Settings.

#include "CollidingPawn.h"
#include "CollidingPawnMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"

// Sets default values
ACollidingPawn::ACollidingPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(40.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	SphereComponent->SetSimulatePhysics(true);

	// Create and position a mesh component so we can see where our sphere is
	UStaticMeshComponent* SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	SphereVisual->SetupAttachment(RootComponent); 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
		SphereVisual->SetWorldScale3D(FVector(0.8f));
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface>SphereMaterial(TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"));
	SphereMaterialWater = SphereMaterial.Object;
	if (SphereMaterial.Succeeded())
	{
		SphereVisual->SetMaterial(0, SphereMaterialWater);
	}

	// Create a particle system that we can activate or deactivate
	OurParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MovementParticles"));
	OurParticleSystem->SetupAttachment(SphereVisual);
	OurParticleSystem->bAutoActivate = false;
	OurParticleSystem->SetRelativeLocation(FVector(-20.0f, 0.0f, 20.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (ParticleAsset.Succeeded())
	{
		OurParticleSystem->SetTemplate(ParticleAsset.Object);
	}

	// Use a spring arm to give the camera smooth, natural-feeling motion.
	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 400.0f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraLagSpeed = 3.0f;

	// Create a camera and attach to our spring arm
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName);

	// Take control of the default player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create an instance of our movement component, and tell it to update our root component.
	OurMovementComponent = CreateDefaultSubobject<UCollidingPawnMovementComponent>(TEXT("CustomMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;
}

UPawnMovementComponent* ACollidingPawn::GetMovementComponent() const
{
	return OurMovementComponent;
}

// Called when the game starts or when spawned
void ACollidingPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACollidingPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Powiekszanie jesli przycisk przyblizania jest wlaczony, oddalanie jesli nie jest
	{
		//SetActorLocation(GetActorForwardVector() * DeltaTime * 0.0001f);
		//FVector NewLocation = GetActorLocation();
		//NewLocation += GetActorUpVector() * -DeltaTime * 100.0f;
		//SetActorLocation(NewLocation);

		UE_LOG(LogTemp, Log, TEXT("Key Released"));

		if (bZoomingYN)
		{
			ZoomFactor += DeltaTime / 0.5f;			// Przyblizanie
		}
		else
		{
			ZoomFactor -= DeltaTime / 0.25f;		// Oddalanie
		}
		ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);
		// Polacz pole widzenia kamery (FOV) i ramie sprezynowe z uwzglednieniem ZoomFactor
		OurCamera->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomFactor);
		OurCameraSpringArm->TargetArmLength = FMath::Lerp<float>(400.0f, 300.0f, ZoomFactor);
	}

	// Obrot kata nachylenia kamery poprzeczny wraz z aktorem
	{
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw += CameraInput.X;
		SetActorRotation(NewRotation);
	}

	// Obrot kata nachylenia kamery gora dol, po odpuszczeniu kamera patrzy w dol
	{
		FRotator NewRotation = OurCameraSpringArm->GetComponentRotation();
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -80.0f, 80.0f);  //-15.0f
		OurCameraSpringArm->SetWorldRotation(NewRotation);
	}
}

// Called to bind functionality to input
void ACollidingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("ParticleToggle", IE_Pressed, this, &ACollidingPawn::ParticleToggle);

	// Zwiazanie zdarzen z przyciskami ustawionymi w pliku DefaultInput.ini
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACollidingPawn::ZoomIn);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ACollidingPawn::ZoomOut);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACollidingPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACollidingPawn::MoveRight);
	PlayerInputComponent->BindAxis("CameraPitch", this, &ACollidingPawn::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &ACollidingPawn::YawCamera);
}


void ACollidingPawn::MoveForward(float AxisValue)
{
	if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorForwardVector() * AxisValue);
	}
}

void ACollidingPawn::MoveRight(float AxisValue)
{
	if (OurMovementComponent && (OurMovementComponent->UpdatedComponent == RootComponent))
	{
		OurMovementComponent->AddInputVector(GetActorRightVector() * AxisValue);
	}
}

void ACollidingPawn::ParticleToggle()
{
	if (OurParticleSystem && OurParticleSystem->Template)
	{
		OurParticleSystem->ToggleActive();
	}
}

void ACollidingPawn::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void ACollidingPawn::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}

void ACollidingPawn::ZoomIn()
{
	bZoomingYN = true;
}

void ACollidingPawn::ZoomOut()
{
	bZoomingYN = false;
}




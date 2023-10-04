// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "CollidingPawn.generated.h"

UCLASS()
class PIERWSZYPROJEKT_API ACollidingPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollidingPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY()
		class UParticleSystemComponent* OurParticleSystem;

	UPROPERTY()
		class UCollidingPawnMovementComponent* OurMovementComponent;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void ParticleToggle();
	void PitchCamera(float AxisValue);		// Gora dol kamera
	void YawCamera(float AxisValue);		// Lewo, prawo
	void ZoomIn();							// Przyblizanie
	void ZoomOut();							// Oddalanie


	// Poruszajaca sie kamera

protected:
	UPROPERTY(EditAnywhere)
		class USpringArmComponent* OurCameraSpringArm;

	UPROPERTY(EditAnywhere)
		class UCameraComponent* OurCamera;

	FVector2D MovementInput;
	FVector2D CameraInput;
	float ZoomFactor;		// Wspolczynnik powiekszania
	bool bZoomingYN;		// Przyblizanie tak/nie	
	UMaterialInterface* SphereMaterialWater = nullptr;
};

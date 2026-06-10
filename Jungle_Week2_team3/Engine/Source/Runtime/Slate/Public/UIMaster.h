#pragma once
#include "Engine/Source/Runtime/Core/Public/Misc/UnrealString.h"

class SUIMaster
{
  public:
    explicit SUIMaster(FString name);
    virtual ~SUIMaster();

    virtual void Render() = 0;
    virtual void Update(float deltaTime);

    void Show();
    void Hide();
    bool IsVisible() const;

    const FString GetName() const;

  protected:
    FString Name;
    bool    bIsVisible = true;
};

#include "InAndOutNativeWidgets.h"

#include "InAndOutMenuPlayerController.h"
#include "InAndOutPlayerController.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Styling/SlateColor.h"
#include "Fonts/SlateFontInfo.h"

namespace InAndOutWidgetHelpers
{
    UTextBlock* MakeText(UWidgetTree* Tree, const FText& Text, int32 FontSize, const FLinearColor& Color)
    {
        UTextBlock* TextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        TextBlock->SetText(Text);
        TextBlock->SetJustification(ETextJustify::Center);
        TextBlock->SetAutoWrapText(true);
        TextBlock->SetColorAndOpacity(FSlateColor(Color));

        FSlateFontInfo FontInfo = TextBlock->GetFont();
        FontInfo.Size = FontSize;
        TextBlock->SetFont(FontInfo);
        return TextBlock;
    }

    UButton* MakeButton(UWidgetTree* Tree, const FText& Label, UTextBlock*& OutText)
    {
        UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
        Button->SetBackgroundColor(FLinearColor(0.13f, 0.28f, 0.50f, 1.0f));

        OutText = MakeText(Tree, Label, 24, FLinearColor::White);
        Button->SetContent(OutText);
        return Button;
    }

    USizeBox* WrapButton(UWidgetTree* Tree, UButton* Button)
    {
        USizeBox* SizeBox = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        SizeBox->SetWidthOverride(340.0f);
        SizeBox->SetHeightOverride(62.0f);
        SizeBox->SetContent(Button);
        return SizeBox;
    }

    UBorder* MakeFullScreenBackground(UWidgetTree* Tree, UCanvasPanel*& OutRoot, const FLinearColor& Color)
    {
        OutRoot = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        Tree->RootWidget = OutRoot;

        UBorder* Background = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
        Background->SetBrushColor(Color);
        Background->SetHorizontalAlignment(HAlign_Center);
        Background->SetVerticalAlignment(VAlign_Center);
        Background->SetPadding(FMargin(30.0f));

        UCanvasPanelSlot* Slot = OutRoot->AddChildToCanvas(Background);
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        Slot->SetOffsets(FMargin(0.0f));
        return Background;
    }

    void AddVerticalChild(UVerticalBox* Box, UWidget* Child, float TopBottomPadding = 8.0f)
    {
        if (UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child))
        {
            Slot->SetPadding(FMargin(0.0f, TopBottomPadding));
            Slot->SetHorizontalAlignment(HAlign_Center);
        }
    }
}

void UInAndOutMainMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    UCanvasPanel* Root = nullptr;
    UBorder* Background = InAndOutWidgetHelpers::MakeFullScreenBackground(
        WidgetTree,
        Root,
        FLinearColor(0.015f, 0.02f, 0.04f, 1.0f));

    UVerticalBox* MenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    Background->SetContent(MenuBox);

    UTextBlock* Title = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        FText::FromString(TEXT("IN AND OUT")),
        58,
        FLinearColor(0.35f, 0.78f, 1.0f, 1.0f));
    InAndOutWidgetHelpers::AddVerticalChild(MenuBox, Title, 22.0f);

    UTextBlock* Subtitle = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        FText::FromString(TEXT("게임기를 지키기 위한 집안 추격전")),
        22,
        FLinearColor(0.85f, 0.85f, 0.90f, 1.0f));
    InAndOutWidgetHelpers::AddVerticalChild(MenuBox, Subtitle, 18.0f);

    UTextBlock* PlayLabel = nullptr;
    UButton* PlayButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("플레이")),
        PlayLabel);
    PlayButton->OnClicked.AddDynamic(this, &UInAndOutMainMenuWidget::HandlePlayClicked);
    InAndOutWidgetHelpers::AddVerticalChild(MenuBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, PlayButton));

    UTextBlock* ExitLabel = nullptr;
    UButton* ExitButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("게임 종료")),
        ExitLabel);
    ExitButton->OnClicked.AddDynamic(this, &UInAndOutMainMenuWidget::HandleExitClicked);
    InAndOutWidgetHelpers::AddVerticalChild(MenuBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, ExitButton));
}

void UInAndOutMainMenuWidget::HandlePlayClicked()
{
    if (AInAndOutMenuPlayerController* PC = Cast<AInAndOutMenuPlayerController>(GetOwningPlayer()))
    {
        PC->StartGameFromMenu();
    }
}

void UInAndOutMainMenuWidget::HandleExitClicked()
{
    if (AInAndOutMenuPlayerController* PC = Cast<AInAndOutMenuPlayerController>(GetOwningPlayer()))
    {
        PC->QuitGameFromMenu();
    }
}

void UInAndOutIntroWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree || WidgetTree->RootWidget)
    {
        if (StoryText)
        {
            StoryText->SetText(PendingStoryLine);
        }
        return;
    }

    UCanvasPanel* Root = nullptr;
    UBorder* Background = InAndOutWidgetHelpers::MakeFullScreenBackground(
        WidgetTree,
        Root,
        FLinearColor::Black);

    StoryText = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        PendingStoryLine,
        34,
        FLinearColor::White);
    StoryText->SetWrapTextAt(900.0f);
    Background->SetContent(StoryText);
}

void UInAndOutIntroWidget::SetStoryLine(const FText& NewStoryLine)
{
    PendingStoryLine = NewStoryLine;

    if (StoryText)
    {
        StoryText->SetText(PendingStoryLine);
    }
}

void UInAndOutResultWidget::Configure(bool bInVictory, const FText& InMessage)
{
    bVictory = bInVictory;
    ResultMessage = InMessage;
    ApplyResultText();
}

void UInAndOutResultWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree || WidgetTree->RootWidget)
    {
        ApplyResultText();
        return;
    }

    UCanvasPanel* Root = nullptr;
    FadeAlpha = 0.78f;
    FadeBackground = InAndOutWidgetHelpers::MakeFullScreenBackground(
        WidgetTree,
        Root,
        FLinearColor(0.0f, 0.0f, 0.0f, FadeAlpha));

    UVerticalBox* ResultBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    FadeBackground->SetContent(ResultBox);

    ResultTitleText = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        FText::GetEmpty(),
        54,
        FLinearColor::White);
    InAndOutWidgetHelpers::AddVerticalChild(ResultBox, ResultTitleText, 18.0f);

    ResultMessageText = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        FText::GetEmpty(),
        29,
        FLinearColor::White);
    ResultMessageText->SetWrapTextAt(900.0f);
    InAndOutWidgetHelpers::AddVerticalChild(ResultBox, ResultMessageText, 22.0f);

    UTextBlock* RetryLabel = nullptr;
    UButton* RetryButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("다시하기")),
        RetryLabel);
    RetryButton->OnClicked.AddDynamic(this, &UInAndOutResultWidget::HandleRetryClicked);
    InAndOutWidgetHelpers::AddVerticalChild(ResultBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, RetryButton));

    UTextBlock* MenuLabel = nullptr;
    UButton* MenuButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("메인 메뉴")),
        MenuLabel);
    MenuButton->OnClicked.AddDynamic(this, &UInAndOutResultWidget::HandleMainMenuClicked);
    InAndOutWidgetHelpers::AddVerticalChild(ResultBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, MenuButton));

    ApplyResultText();
}

void UInAndOutResultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    FadeAlpha = FMath::FInterpTo(FadeAlpha, 0.88f, InDeltaTime, 3.5f);
    if (FadeBackground)
    {
        FadeBackground->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, FadeAlpha));
    }
}

void UInAndOutResultWidget::ApplyResultText()
{
    if (ResultTitleText)
    {
        ResultTitleText->SetText(FText::FromString(bVictory ? TEXT("승리") : TEXT("패배")));
        ResultTitleText->SetColorAndOpacity(FSlateColor(
            bVictory
                ? FLinearColor(0.35f, 1.0f, 0.55f, 1.0f)
                : FLinearColor(1.0f, 0.25f, 0.30f, 1.0f)));
    }

    if (ResultMessageText)
    {
        ResultMessageText->SetText(ResultMessage);
    }
}

void UInAndOutResultWidget::HandleRetryClicked()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(GetOwningPlayer()))
    {
        PC->RestartCurrentBattle();
    }
}

void UInAndOutResultWidget::HandleMainMenuClicked()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(GetOwningPlayer()))
    {
        PC->ReturnToMainMenu();
    }
}

void UInAndOutPauseWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    UCanvasPanel* Root = nullptr;
    UBorder* Background = InAndOutWidgetHelpers::MakeFullScreenBackground(
        WidgetTree,
        Root,
        FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));

    UVerticalBox* PauseBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    Background->SetContent(PauseBox);

    UTextBlock* Title = InAndOutWidgetHelpers::MakeText(
        WidgetTree,
        FText::FromString(TEXT("일시정지")),
        48,
        FLinearColor::White);
    InAndOutWidgetHelpers::AddVerticalChild(PauseBox, Title, 20.0f);

    UTextBlock* ResumeLabel = nullptr;
    UButton* ResumeButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("계속하기")),
        ResumeLabel);
    ResumeButton->OnClicked.AddDynamic(this, &UInAndOutPauseWidget::HandleResumeClicked);
    InAndOutWidgetHelpers::AddVerticalChild(PauseBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, ResumeButton));

    UTextBlock* RetryLabel = nullptr;
    UButton* RetryButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("다시하기")),
        RetryLabel);
    RetryButton->OnClicked.AddDynamic(this, &UInAndOutPauseWidget::HandleRetryClicked);
    InAndOutWidgetHelpers::AddVerticalChild(PauseBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, RetryButton));

    UTextBlock* MenuLabel = nullptr;
    UButton* MenuButton = InAndOutWidgetHelpers::MakeButton(
        WidgetTree,
        FText::FromString(TEXT("메인 메뉴")),
        MenuLabel);
    MenuButton->OnClicked.AddDynamic(this, &UInAndOutPauseWidget::HandleMainMenuClicked);
    InAndOutWidgetHelpers::AddVerticalChild(PauseBox, InAndOutWidgetHelpers::WrapButton(WidgetTree, MenuButton));
}

void UInAndOutPauseWidget::HandleResumeClicked()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(GetOwningPlayer()))
    {
        PC->ResumeBattle();
    }
}

void UInAndOutPauseWidget::HandleRetryClicked()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(GetOwningPlayer()))
    {
        PC->RestartCurrentBattle();
    }
}

void UInAndOutPauseWidget::HandleMainMenuClicked()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(GetOwningPlayer()))
    {
        PC->ReturnToMainMenu();
    }
}

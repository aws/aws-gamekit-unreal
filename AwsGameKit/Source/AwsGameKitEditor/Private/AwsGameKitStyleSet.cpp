// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "AwsGameKitStyleSet.h"

// Unreal
#include <Interfaces/IPluginManager.h>
#include <Styling/SlateStyle.h>
#include <Styling/SlateStyleRegistry.h>
#include <Styling/SlateTypes.h>

TSharedPtr<FSlateStyleSet> AwsGameKitStyleSet::Style;

AwsGameKitStyleSet::AwsGameKitStyleSet()
{
    if (Style.IsValid())
    {
        return;
    }

    Style = MakeShareable(new FSlateStyleSet("AwsGameKitStyle"));
    Style->SetContentRoot(IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir() / TEXT("Resources") / TEXT("icons"));

    // Fonts
    Style->Set("RobotoRegular8", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 8));
    Style->Set("RobotoRegular10", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 10));
    Style->Set("RobotoRegular12", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 12));
    Style->Set("RobotoBold10", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 10));
    Style->Set("RobotoBold11", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 11));
    Style->Set("RobotoBold12", FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12));

    // Colors
    Style->Set("ButtonGreen", FColor::FromHex("#2F8C00"));
    Style->Set("ButtonGrey", FColor::FromHex("#EEEEEE"));
    Style->Set("ButtonRed", FColor::FromHex("#CC0000"));
    Style->Set("BackgroundGrey", FColor::FromHex("#333333"));
    Style->Set("ModalDialogBackground", FColor::FromHex("#3E3E3E"));
    Style->Set("Black", FColor::FromHex("#000000"));
    Style->Set("DarkGrey", FColor::FromHex("#191919"));
    Style->Set("MediumGrey", FColor::FromHex("#666666"));
    Style->Set("TextMediumGrey", FColor::FromHex("#AAAAAA"));
    Style->Set("LightGrey", FColor::FromHex("#CCCCCC"));
    Style->Set("White", FColor::FromHex("#FCFCFC"));
    Style->Set("ErrorRed", FColor::FromHex("#D13212"));
    Style->Set("InfoBlue", FColor::FromHex("#0073D9"));

    // Brushes
    FSlateColorBrush* darkGreyBrush = new FSlateColorBrush(Style->GetColor("DarkGrey"));
    Style->Set("DarkGreyBrush", darkGreyBrush);
    FSlateColorBrush* mediumGreyBrush = new FSlateColorBrush(Style->GetColor("MediumGrey"));
    Style->Set("MediumGreyBrush", mediumGreyBrush);
    FSlateColorBrush* backgroundGreyBrush = new FSlateColorBrush(Style->GetColor("BackgroundGrey"));
    Style->Set("BackgroundGreyBrush", backgroundGreyBrush);
    FSlateColorBrush* backgroundModalDialog = new FSlateColorBrush(Style->GetColor("ModalDialogBackground"));
    Style->Set("BackgroundModalDialogBrush", backgroundModalDialog);
    FSlateColorBrush* errorRedBrush = new FSlateColorBrush(Style->GetColor("ErrorRed"));
    Style->Set("ErrorRedBrush", errorRedBrush);
    FSlateColorBrush* infoBlueBrush = new FSlateColorBrush(Style->GetColor("InfoBlue"));
    Style->Set("InfoBlueBrush", infoBlueBrush);

    // Text
    const FTextBlockStyle& NormalText = FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");

    Style->Set("DescriptionText", FTextBlockStyle(NormalText)
        .SetFont(Style->GetFontStyle("RobotoRegular10"))
        .SetColorAndOpacity(Style->GetColor("TextMediumGrey")));

    Style->Set("DescriptionBoldText", FTextBlockStyle(NormalText)
        .SetFont(Style->GetFontStyle("RobotoBold10")));

    Style->Set("ModalDialogText", FTextBlockStyle(NormalText)
        .SetFont(FEditorStyle::GetFontStyle("StandardDialog.LargeFont"))
        .SetColorAndOpacity(Style->GetColor("White")));
        
    Style->Set("Button.WhiteText", FTextBlockStyle(NormalText)
        .SetFont(Style->GetFontStyle("RobotoBold10"))
        .SetColorAndOpacity(Style->GetColor("White"))
        .SetShadowOffset(FVector2D(1.0f, 1.0f)));

    Style->Set("Button.NormalText", FTextBlockStyle(NormalText));

    // Hyperlinks
    Style->Set("Hyperlink", FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>("Hyperlink"));

    FHyperlinkStyle modalDialogHyperLink = FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>("Hyperlink");
    modalDialogHyperLink.TextStyle.Font = FEditorStyle::GetFontStyle("StandardDialog.LargeFont");
    Style->Set("ModalHyperlink", modalDialogHyperLink);

    // Icons
    FSlateImageBrush* deployedIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("success.png")), FVector2D(15, 15));
    Style->Set("DeployedIcon", deployedIconBrush);

    FSlateImageBrush* waitingIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("waiting.png")), FVector2D(15, 15));
    Style->Set("WaitingIcon", waitingIconBrush);

    FSlateImageBrush* errorIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("error.png")), FVector2D(15, 15));
    Style->Set("ErrorIcon", errorIconBrush);

    FSlateImageBrush* progressIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("working.png")), FVector2D(15, 15));
    Style->Set("ProgressIcon", progressIconBrush);

    FSlateImageBrush* unsynchronizedIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("unsynchronized.png")), FVector2D(15, 15));
    Style->Set("UnsynchronizedIcon", unsynchronizedIconBrush);

    FSlateImageBrush* deleteIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("garbage.png")), FVector2D(15,15));
    Style->Set("DeleteIcon", deleteIconBrush);

    FSlateImageBrush* cloudIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("cloud.png")), FVector2D(20,15));
    Style->Set("CloudIcon", cloudIconBrush);

    FSlateImageBrush* warningIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("warning.png")), FVector2D(20,20));
    Style->Set("WarningIcon", warningIconBrush);

    FSlateImageBrush* warningIconBrushSmall = new FSlateImageBrush(Style->RootToContentDir(TEXT("warning_16x16.png")), FVector2D(12, 12));
    Style->Set("WarningIconSmall", warningIconBrushSmall);

    FSlateImageBrush* warningIconBrushInline = new FSlateImageBrush(Style->RootToContentDir(TEXT("warning_inline.png")), FVector2D(10, 10));
    Style->Set("WarningIconInline", warningIconBrushInline);

    FSlateImageBrush* externalIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("external.png")), FVector2D(20, 20));
    Style->Set("ExternalIcon", externalIconBrush);

    FSlateImageBrush* refreshIconBrush = new FSlateImageBrush(Style->RootToContentDir(TEXT("refresh.png")), FVector2D(15, 15));
    Style->Set("RefreshIcon", refreshIconBrush);

    // Inline text image
    FInlineTextImageStyle inlineTextImageStyle = FInlineTextImageStyle()
        .SetImage(*warningIconBrushInline);
    Style->Set("WarningIconInline", inlineTextImageStyle);

    FButtonStyle helpButtonStyle = FButtonStyle()
        .SetPressed(*FEditorStyle::GetBrush("HelpIcon.Pressed"))
        .SetNormal(*FEditorStyle::GetBrush("HelpIcon"))
        .SetHovered(*FEditorStyle::GetBrush("HelpIcon.Hovered"));
    Style->Set("HelpButtonStyle", helpButtonStyle);

    FSlateStyleRegistry::RegisterSlateStyle(*Style.Get());
}

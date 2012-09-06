#import <Cocoa/Cocoa.h>
@class GuiTree;
@class ToolTreeSet;
@class TreeSetView;




@interface WindowControllerTreeViewer : NSWindowController {

    IBOutlet NSTextField*           treeCounter;
    IBOutlet NSStepper*             treeStepper;
    IBOutlet NSButton*              closeButton;
    IBOutlet TreeSetView*           treeView;
    IBOutlet NSTextField*           fontLabel;
    IBOutlet NSTextField*           fontEntry;
    IBOutlet NSTextField*           treeInfo;
    IBOutlet NSPopUpButton*         outgroupList;
    IBOutlet NSTextField*           outgroupLabel;
    ToolTreeSet*                    myTool;
    int                             selectedTree;
    float                           fontSize;
}

@property (readwrite) int selectedTree;
@property (readwrite) float fontSize;

- (IBAction)changeFontSize:(id)sender;
- (IBAction)changeOutgroup:(id)sender;
- (IBAction)closeButtonAction:(id)sender;
- (IBAction)helpButtonAction:(id)sender;
- (GuiTree*)activeTree;
- (id)initWithTool:(ToolTreeSet*)t;
- (void)initializeTreeInformation;
- (void)populateOutgroupList;
- (IBAction)stepAction:(id)sender;

@end

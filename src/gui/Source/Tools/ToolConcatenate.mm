#import "Inlet.h"
#import "RevBayes.h"
#import "ToolConcatenate.h"
#import "WindowControllerConcatenate.h"




@implementation ToolConcatenate

@synthesize useMinimalSet;
@synthesize matchUsingNames;

- (void)awakeFromNib {

}

- (void)closeControlPanel {

    [NSApp stopModal];
	[controlWindow close];
}

- (void)dealloc {

	[controlWindow release];
	[super dealloc];
}

- (void)encodeWithCoder:(NSCoder *)aCoder {

	[aCoder encodeBool:useMinimalSet   forKey:@"useMinimalSet"];
	[aCoder encodeBool:matchUsingNames forKey:@"matchUsingNames"];

	[super encodeWithCoder:aCoder];
}

- (id)init {

    self = [self initWithScaleFactor:1.0];
    return self;
}

- (id)initWithScaleFactor:(float)sf {

    if ( (self = [super initWithScaleFactor:sf]) ) 
		{
		// initialize the tool image
		[self initializeImage];
        [self setImageWithSize:itemSize];
		
		// initialize the inlet/outlet information
		[self addInletOfColor:[NSColor greenColor]];
		[self addOutletOfColor:[NSColor greenColor]];
        [self setInletLocations];
        [self setOutletLocations];

		// initialize some variables
        useMinimalSet   = YES;
        matchUsingNames = YES;
		
		// initialize the control window
		controlWindow = [[WindowControllerConcatenate alloc] initWithTool:self];
		}
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {

    if ( (self = [super initWithCoder:aDecoder]) ) 
		{
		// initialize the tool image
		[self initializeImage];
        [self setImageWithSize:itemSize];
        
        useMinimalSet   = [aDecoder decodeBoolForKey:@"useMinimalSet"];
        matchUsingNames = [aDecoder decodeBoolForKey:@"matchUsingNames"];

		// initialize the control window
		controlWindow = [[WindowControllerConcatenate alloc] initWithTool:self];
		}
	return self;
}

- (void)initializeImage {

    itemImage[0] = [NSImage imageNamed:@"Tool_Concatenate25.icns"];
    itemImage[1] = [NSImage imageNamed:@"Tool_Concatenate50.icns"];
    itemImage[2] = [NSImage imageNamed:@"Tool_Concatenate75.icns"];
    itemImage[3] = [NSImage imageNamed:@"Tool_Concatenate100.icns"];
    itemImage[4] = [NSImage imageNamed:@"Tool_Concatenate125.icns"];
    itemImage[5] = [NSImage imageNamed:@"Tool_Concatenate150.icns"];
    itemImage[6] = [NSImage imageNamed:@"Tool_Concatenate200.icns"];
    itemImage[7] = [NSImage imageNamed:@"Tool_Concatenate400.icns"];

	float s[8] = { 0.25, 0.50, 0.75, 1.0, 1.25, 1.50, 2.0, 4.0 };
	for (int i=0; i<8; i++)
        [itemImage[i] setSize:NSMakeSize(ITEM_IMAGE_SIZE*s[i], ITEM_IMAGE_SIZE*s[i])];
}

- (int)numberOfIncomingAlignments {

    return 0;
}

- (int)mininumNumberOfIncomingSequences {

    return 0;
}

- (int)maximumNumberOfIncomingSequences {

    return 0;
}

- (NSMutableAttributedString*)sendTip {

    NSString* myTip = [NSString stringWithString:@" Sequence Concatenation Tool "];
    if ([self isResolved] == YES)
        myTip = [myTip stringByAppendingFormat:@"\n Status: Resolved \n # Matrices: %d ", [self numDataMatrices]];
    else 
        myTip = [myTip stringByAppendingString:@"\n Status: Unresolved "];
    if ([self isFullyConnected] == YES)
        myTip = [myTip stringByAppendingString:@"\n Fully Connected "];
    else 
        myTip = [myTip stringByAppendingString:@"\n Missing Connections "];

    NSDictionary *attr = [NSDictionary 
                 dictionaryWithObjects:[NSArray arrayWithObjects:[NSFont fontWithName:@"Lucida Grande Bold" size:14.0], [[NSColor whiteColor] colorWithAlphaComponent:1.0], nil] 
                               forKeys: [NSArray arrayWithObjects:NSFontAttributeName,NSForegroundColorAttributeName, nil]];

    NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc] initWithString:myTip attributes:attr];

    return attrString;
}

- (void)showControlPanel {

    NSPoint p = [self originForControlWindow:[controlWindow window]];
    [[controlWindow window] setFrameOrigin:p];
	//[controlWindow setMatrixList];
	[controlWindow showWindow:self];    
	[[controlWindow window] makeKeyAndOrderFront:nil];
    [NSApp runModalForWindow:[controlWindow window]];
}

- (void)updateForChangeInState {

    [self startProgressIndicator];
    
    // set the tool state to unresolved
    [self setIsResolved:NO];
    
    // set up an array of outlets from parent tools
    NSMutableArray* dataOutlets = [NSMutableArray arrayWithCapacity:1];
    for (int i=0; i<[self numInlets]; i++)
        {
        Inlet* theInlet = [self inletIndexed:i];
        Tool* t = [self getParentToolOfInlet:theInlet];
        if (t != nil)
            {
            if ( [t isKindOfClass:[ToolData class]] == YES )
                {
                [dataOutlets addObject:[theInlet getMyOutlet]];
                }
            }
        }
        
	// update the state of this tool depending upon the state/presence of the parent tool
	if ( [dataOutlets count] == 0 )
		{
		// we don't have a parent tool that contains data
		[self removeAllDataMatrices];
		}
	else 
		{
		// we have parent data-containing tool(s)
        
        // for the hell of it, we'll delete everything and freshly concatenate the sequences
		[self removeAllDataMatrices];

        // check to see if our current data is simply a copy of the data in the parents, in which case
        // we don't need to do anything
        NSMutableArray* parentDataMatrices = [NSMutableArray arrayWithCapacity:1];
        for (int i=0; i<[dataOutlets count]; i++)
            {
            Outlet* ol = [dataOutlets objectAtIndex:i];
            ToolData* t = (ToolData*)[ol toolOwner];
            if ( [ol toolColor] == [NSColor greenColor] )
                {
                [parentDataMatrices addObjectsFromArray:[t getAlignedData]];
                }
            }
            
        
        
        int numFound = 0;
        for (int i=0; i<[dataMatrices count]; i++)
            {
            RbData* myDataMatrix = [dataMatrices objectAtIndex:i];
            for (int j=0; j<[parentDataMatrices count]; j++)
                {
                RbData* parentDataMatrix = [parentDataMatrices objectAtIndex:j];
                if ( parentDataMatrix == [myDataMatrix copiedFrom] )
                    {
                    numFound++;
                    break;
                    }
                }
            }
            
        // remove all of the data matrices if each and every data matrix in this tool is not
        // a copy of the data matrices in the parents
        if ( [parentDataMatrices count] != numFound || [parentDataMatrices count] != [dataMatrices count] )
            {
            [self removeAllDataMatrices];
            for (int i=0; i<[parentDataMatrices count]; i++)
                {
				RbData* d = [parentDataMatrices objectAtIndex:i];
				RbData* nd = [[RbData alloc] initWithRbData:d];
				[self addMatrix:nd];
                }
            }
            
        if ( [dataMatrices count] > 0 )
            {
            [self setIsResolved:YES];
            [self makeDataInspector];
            }
		}
                
    [self stopProgressIndicator];
}

@end
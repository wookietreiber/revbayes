#import <Cocoa/Cocoa.h>
#import "ParmRandomVariable.h"



@interface ParmRealPos : ParmRandomVariable <NSCoding> {

}

- (id)initWithScaleFactor:(float)sf andTool:(ToolModel*)t;

@end

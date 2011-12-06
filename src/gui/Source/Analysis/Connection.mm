#import "Connection.h"
#import "Inlet.h"
#import "Outlet.h"



@implementation Connection

@synthesize inlet;
@synthesize outlet;
@synthesize isSelected;

- (void)encodeWithCoder:(NSCoder*)aCoder {

    [aCoder encodeBool:isSelected forKey:@"isSelected"];
	[aCoder encodeObject:inlet    forKey:@"inlet"];
	[aCoder encodeObject:outlet   forKey:@"outlet"];
}

- (void)flipSelected {

	if (isSelected == YES)
		isSelected = NO;
	else 
		isSelected = YES;
}

- (id)init {

    if ( (self = [super init]) ) 
        {
        inlet = nil;
        outlet = nil;
        isSelected = NO;
        }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {

    if ( (self = [super init]) ) 
		{
		isSelected = [aDecoder decodeBoolForKey:@"isSelected"];
		inlet      = [aDecoder decodeObjectForKey:@"inlet"];
		outlet     = [aDecoder decodeObjectForKey:@"outlet"];
		}
	return self;
}

@end
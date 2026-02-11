#import "UIKit/UIKit.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end


@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

	self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	UIViewController *vc = [[UIViewController alloc] init];
	vc.view.backgroundColor = [UIColor whiteColor];

	UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(40, 200, 300, 40)];
	label.text = @"Hello Objective-C iOS App";
	label.textColor = [UIColor blackColor];

	[vc.view addSubview:label];

	self.window.rootViewController = vc;
	[self.window makeKeyAndVisible];

	return YES;
}
@end

int main( int c, char **v )
{
	@autoreleasepool
	{
		return UIApplicationMain(c, v, nil, NSStringFromClass([AppDelegate class]));
	}
};

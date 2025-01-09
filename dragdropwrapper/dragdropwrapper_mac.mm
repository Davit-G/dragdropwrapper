#import "dragdropwrapper.h"

#import <Foundation/Foundation.h>
#import "Cocoa/Cocoa.h"

@interface CustomDraggingSource : NSObject <NSDraggingSource>

@property (nonatomic, copy) void (^callback)(void);

@end

@implementation CustomDraggingSource

- (instancetype)initWithCallback:(void (^)(void))callback {
    self = [super init];
    if (self) {
        _callback = [callback copy];
    }
    return self;
}

- (NSDragOperation)draggingSession:(nonnull NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
    return NSDragOperationCopy;
}

// Call this method when the dragging operation is done
- (void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation {
    NSLog(@"Drag session ended at: (%f, %f) with operation: %ld", screenPoint.x, screenPoint.y, (long)operation);
    if (self.callback) {
        self.callback();
    }
}

@end

extern "C" {
    void SendFileAsDragDrop(void* handle, const char* file_path, std::function<void(void)> callback) {
        NSWindow *window = (__bridge NSWindow*)handle; // handles used by other libs are usually NSWindows
        NSView *view = [window contentView]; // get the root view of the window
        
        @autoreleasepool {
            if (auto event = [[view window] currentEvent])
            {
                NSString* filename = (NSString* _Nonnull)[NSString stringWithCString:file_path encoding:NSStringEncodingConversionAllowLossy];
                NSURL *fileURL = [NSURL fileURLWithPath: filename];
                NSDraggingItem *dragItem = [[NSDraggingItem alloc] initWithPasteboardWriter: fileURL];

                auto eventLocation = [event locationInWindow];
                NSRect dragRect = [view convertRect: NSMakeRect (eventLocation.x - 20, eventLocation.y - 20, 40.f, 40.f)
                                        fromView: nil];

                auto icon = [[NSWorkspace sharedWorkspace] iconForFile: filename];
                [dragItem setDraggingFrame: dragRect contents: icon];

                auto dragItems = [[NSMutableArray alloc] init];
                [dragItems addObject: dragItem];

                void (^objcCallback)(void) = [callback]() {
                    callback();
                };
                
                CustomDraggingSource *nsDragSource = [[CustomDraggingSource alloc] initWithCallback: objcCallback];
                
                [view beginDraggingSessionWithItems: dragItems
                                                    event: event
                                                    source: nsDragSource];
            }
        }
    }
}

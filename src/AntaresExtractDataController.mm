// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "AntaresExtractDataController.h"

#include "sfz/sfz.hpp"
#include "DataExtractor.hpp"

using antares::DataExtractor;
using sfz::Bytes;
using sfz::Exception;
using sfz::Rune;
using sfz::String;
using sfz::StringPiece;
using sfz::format;
using sfz::print;

namespace io = sfz::io;
namespace utf8 = sfz::utf8;

#define VERIFYING @"Verifying scenario data..."
#define EXTRACTING @"Extracting scenario data..."

namespace {

class NSLogTarget {
  public:
    NSLogTarget() { }
    void append(const char* string) { _buffer.append(string); flush(); }
    void append(const String& string) { _buffer.append(string); flush(); }
    void append(const StringPiece& string) { _buffer.append(string); flush(); }
    void append(size_t num, Rune rune) { _buffer.append(num, rune); flush(); }
  private:
    void flush() {
        while (_buffer.find('\n') != String::npos) {
            size_t split = _buffer.find('\n');
            Bytes encoded(utf8::encode(_buffer.substr(0, split)));
            _buffer.assign(_buffer.substr(split + 1));
            NSString* nsstring = [[NSString alloc] initWithBytes:encoded.data()
                length:encoded.size() encoding:NSUTF8StringEncoding];
            NSLog(@"%@", nsstring);
        }
    }
    String _buffer;
    DISALLOW_COPY_AND_ASSIGN(NSLogTarget);
};
NSLogTarget nslog;

template <typename T>
class Scoped {
  public:
    Scoped(T* t) : _t(t) { }
    ~Scoped() { [_t release]; }
    T* get() const { return _t; }
  private:
    T* _t;
};

}  // namespace

@implementation AntaresExtractDataController

- (id)initWithTarget:(id)target selector:(SEL)selector {
    if (!(self = [super init])) {
        return nil;
    }
    _target = target;
    _selector = selector;
    if (![NSBundle loadNibNamed:@"ExtractData" owner:self]) {
        [self release];
        return nil;
    }
    return self;
}

- (void)awakeFromNib {
    NSString* label = NSLocalizedStringFromTable(VERIFYING, @"ExtractData", VERIFYING);
    [_status_field setStringValue:label];
    [_progress_bar startAnimation:self];
    [_window center];
    [_window makeKeyAndOrderFront:self];
    [NSThread detachNewThreadSelector:@selector(doWork) toTarget:self withObject:nil];
}

- (void)extracting {
    NSString* label = NSLocalizedStringFromTable(EXTRACTING, @"ExtractData", EXTRACTING);
    [_status_field setStringValue:label];
}

- (void)done {
    [_window close];
    [_target performSelector:_selector withObject:self];
    [self release];
}

- (void)doWork {
    Scoped<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);

    const String home(utf8::decode(getenv("HOME")));
    const String application_support(format("{0}/Library/Application Support/Antares", home));
    String source(format("{0}/Downloads", application_support));
    String dest(format("{0}/Scenarios/com.biggerplanet.ares", application_support));

    DataExtractor extractor(source, dest);
    if (!extractor.current()) {
        [self performSelectorOnMainThread:@selector(extracting) withObject:nil waitUntilDone:NO];
        extractor.extract(&nslog);
    }
    [self performSelectorOnMainThread:@selector(done) withObject:nil waitUntilDone:NO];
}

@end

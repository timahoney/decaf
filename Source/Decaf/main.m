/*
 * Copyright (C) 2013 Tim Mahoney
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // We want to force the MiniBrowser to use the bundled
    // WebCore and WebKit. We can do this with DYLD_FRAMEWORK_PATH.
    NSString *frameworkPath = [[NSBundle mainBundle] privateFrameworksPath];
    NSString *miniBrowserPath = [[NSBundle mainBundle] pathForResource:@"Decaf" ofType:@"app"];
    NSBundle *miniBrowserBundle = [NSBundle bundleWithPath:miniBrowserPath];
    
    NSMutableDictionary *environment = [[[NSDictionary dictionaryWithObjectsAndKeys:
                                         frameworkPath, @"DYLD_FRAMEWORK_PATH",
                                         @"YES", @"WEBKIT_UNSET_DYLD_FRAMEWORK_PATH", nil] mutableCopy] autorelease];
    [environment addEntriesFromDictionary:[[NSProcessInfo processInfo] environment]];
    
    NSDictionary *configuration = [NSDictionary dictionaryWithObject:environment forKey:NSWorkspaceLaunchConfigurationEnvironment];
    NSURL *executableURL = [NSURL fileURLWithPath:[miniBrowserBundle executablePath]];
    [[NSWorkspace sharedWorkspace] launchApplicationAtURL:executableURL options:0 configuration:configuration error:0];
    
    [pool release];
    return 0;
}

#import <Foundation/Foundation.h>
#import "../EnvExt.hpp"

namespace UI { namespace App {

const char * EnvExt::documentPath(void) const {
    if (m_documentPath.empty()) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectoryPath = [paths objectAtIndex:0];
        m_documentPath =  [documentsDirectoryPath UTF8String];
    }

	return m_documentPath.c_str();
}

}}

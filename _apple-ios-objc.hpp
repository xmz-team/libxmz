/*
 * _apple-ios-objc.hpp
 * Copyright (c) 2026 XMZ Studio
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#ifndef _LIBXMZ_IOS_APPLE_OBJC_HPP
#define _LIBXMZ_IOS_APPLE_OBJC_HPP
#if defined(__OBJC__) && defined(__APPLE__)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace xmz::ios {
	namespace objc::alert::_ {
		/*
		* Accept any button configuration
		* titles: button title array
		* cancelIndex: Which button is Cancel style (-1 means none)
		* completion: callback, the parameter is the index of the clicked button
		*/
		inline void
		present(UIViewController* vc, NSString* title, NSString* message,
				NSArray<NSString*>* titles, NSInteger cancelIndex,
				void (^completion)(NSInteger index))
		{
			UIAlertController* alert = [UIAlertController alertControllerWithTitle:title
										message:message
										preferredStyle:UIAlertControllerStyleAlert];
			for (NSInteger i = 0; i < (NSInteger)titles.count; ++i) {
				UIAlertActionStyle style = (i == cancelIndex) ? UIAlertActionStyleCancel : UIAlertActionStyleDefault;
				UIAlertAction* action = [UIAlertAction actionWithTitle:titles[i]
										 style:style
										 handler:^(UIAlertAction* a) {
											 if (completion) completion(i);
										 }];
				[alert addAction:action];
			}
			[vc presentViewController:alert animated:YES completion:nil];
		}
	} /* namespace objc::alert::_ */
} /* namespace xmz */
#else
#error "only apple platform and use objc/objc++ code, only then can it be used"
#endif /* __OBJC__ && __APPLE__ */
#endif /* _LIBXMZ_IOS_APPLE_OBJC_HPP */


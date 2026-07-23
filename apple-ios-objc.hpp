/*
 * apple-ios-objc.hpp
 * Created by XMZ <xmz-team@outlook.com> on 21/7/26
 * Copyright (c) 2026 XMZ <xmz-team@outlook.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
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

#ifndef XMZ_TEAM_IOS_APPLE_OBJC_HPP
#define XMZ_TEAM_IOS_APPLE_OBJC_HPP
#if defined(__OBJC__) && defined(__APPLE__)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#else
#error "only apple platform and use objc/objc++ code, only then can it be used"
#endif // __OBJC__ && __APPLE__

namespace xmz {
#if defined(__OBJC__) && defined(__APPLE__)
    namespace objc::ui {
        // MARK: simple application entrance
        inline int run(int argc, char* argv[], id delegate = nil) {
            @autoreleasepool {
                return UIApplicationMain(
                    argc, argv, nil, 
                    delegate ? NSStringFromClass(
                        [delegate class]
                    ) : nil
                );
            }
        }
    } // namespace objc::ui
    // MARK: window establishment assistance
    namespace objc::make {
        inline UIWindow* window() { return [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]; }
        inline UILabel* label(NSString* text) {
            UILabel* label = [[UILabel alloc] init];
            label.text = text;
            label.numberOfLines = 0;
            label.textAlignment = NSTextAlignmentCenter;
            label.translatesAutoresizingMaskIntoConstraints = NO;
            [
                label setContentCompressionResistancePriority:UILayoutPriorityRequired 
                forAxis:UILayoutConstraintAxisVertical
            ];
            [
                label setContentHuggingPriority:UILayoutPriorityRequired 
                forAxis:UILayoutConstraintAxisVertical
            ];
            return label;
        }
        inline UILabel* label(NSString* prefix, NSString* text) {
            NSString* combined = [NSString stringWithFormat:@"%@%@", prefix, text];
            return label(combined);
        }
        inline UIButton* button(NSString* title) {
            UIButton* button = [
                UIButton 
                buttonWithType:UIButtonTypeSystem
            ];
            [button setTitle:title forState:UIControlStateNormal];
            button.translatesAutoresizingMaskIntoConstraints = NO;
            return button;
        }
        inline UINavigationController* navigationController(UIViewController* root) { return [[UINavigationController alloc] initWithRootViewController:root]; }
        inline constexpr auto& navctrl = navigationController;
        inline constexpr auto& navctl = navigationController;
        // add UIImageView
        inline UIImageView* imageView(UIImage* image) {
            UIImageView* imageView = [[UIImageView alloc] initWithImage:image];
            imageView.contentMode = UIViewContentModeScaleAspectFit;
            imageView.translatesAutoresizingMaskIntoConstraints = NO;
            return imageView;
        }
        inline UIImageView* imageView(NSString* imageName) {
            UIImage* image = [UIImage imageNamed:imageName];
            return imageView(image);
        }
        inline auto imgview = [](auto&& arg) { return imageView(std::forward<decltype(arg)>(arg)); };
        // add UITextField
        inline UITextField* textField(NSString* placeholder = nil) {
            UITextField* textField = [[UITextField alloc] init];
            textField.borderStyle = UITextBorderStyleRoundedRect;
            textField.placeholder = placeholder;
            textField.translatesAutoresizingMaskIntoConstraints = NO;
            return textField;
        }
        inline constexpr auto& textfield = textField;
        // add UISwitch
        inline UISwitch* toggle() {
            UISwitch* toggle = [[UISwitch alloc] init];
            toggle.translatesAutoresizingMaskIntoConstraints = NO;
            return toggle;
        }
        // add UIScrollView
        inline UIScrollView* scrollView() {
            UIScrollView* scrollView = [[UIScrollView alloc] init];
            scrollView.translatesAutoresizingMaskIntoConstraints = NO;
            return scrollView;
        }
        inline constexpr auto& scrollview = scrollView;
        // add UIStackView
        inline UIStackView* stackView(NSArray* arrangedSubviews = nil) {
            UIStackView* stackView = [[UIStackView alloc] initWithArrangedSubviews:arrangedSubviews];
            stackView.axis = UILayoutConstraintAxisVertical;
            stackView.spacing = 10;
            stackView.distribution = UIStackViewDistributionFill;
            stackView.alignment = UIStackViewAlignmentCenter;
            stackView.translatesAutoresizingMaskIntoConstraints = NO;
            return stackView;
        }
        inline constexpr auto& stackview = stackView;
    } // namespace objc::make
    namespace objc::mk = objc::make;
    // MARK: automatic layout assistance
    namespace objc::layout {
        inline void center(UIView* view, UIView* inView) {
            view.translatesAutoresizingMaskIntoConstraints = NO;
            NSMutableArray* constraints = [NSMutableArray array];
            // central constraint
            [constraints addObject:[view.centerXAnchor constraintEqualToAnchor:inView.centerXAnchor]];
            [constraints addObject:[view.centerYAnchor constraintEqualToAnchor:inView.centerYAnchor]];
            // special treatment of UILabel
            if ([view isKindOfClass:[UILabel class]]) {
                UILabel* label = (UILabel*)view;
                label.numberOfLines = 0;
                label.textAlignment = NSTextAlignmentCenter;
                [label setContentCompressionResistancePriority:UILayoutPriorityRequired 
                                                       forAxis:UILayoutConstraintAxisVertical];
                [label setContentHuggingPriority:UILayoutPriorityRequired 
                                         forAxis:UILayoutConstraintAxisVertical];
                 // add margin constraints
                [constraints addObject:[label.leadingAnchor constraintGreaterThanOrEqualToAnchor:inView.leadingAnchor constant:20]];
                [constraints addObject:[label.trailingAnchor constraintLessThanOrEqualToAnchor:inView.trailingAnchor constant:-20]];
                [constraints addObject:[label.widthAnchor constraintLessThanOrEqualToAnchor:inView.widthAnchor constant:-40]];
            }
            [NSLayoutConstraint activateConstraints:constraints];
        }
        // an enhanced version of the center, which can customize the margin
        inline void center(UIView* view, UIView* inView, CGFloat horizontalPadding, CGFloat verticalPadding) {
            view.translatesAutoresizingMaskIntoConstraints = NO;
            NSMutableArray* constraints = [NSMutableArray arrayWithArray:@[
                [view.centerXAnchor constraintEqualToAnchor:inView.centerXAnchor],
                [view.centerYAnchor constraintEqualToAnchor:inView.centerYAnchor],
                [view.widthAnchor constraintLessThanOrEqualToAnchor:inView.widthAnchor constant:-horizontalPadding * 2]
            ]];
            // UILabel special treatment
            if ([view isKindOfClass:[UILabel class]]) {
                UILabel* label = (UILabel*)view;
                label.numberOfLines = 0;
                label.textAlignment = NSTextAlignmentCenter;
                [label setContentCompressionResistancePriority:UILayoutPriorityRequired 
                                                       forAxis:UILayoutConstraintAxisVertical];
                [label setContentHuggingPriority:UILayoutPriorityRequired 
                                         forAxis:UILayoutConstraintAxisVertical];
            }
            [NSLayoutConstraint activateConstraints:constraints];
        }
        inline void fill(UIView* view, UIView* inView, CGFloat padding = 0) {
            [NSLayoutConstraint activateConstraints:@[
                [view.topAnchor constraintEqualToAnchor:inView.topAnchor constant:padding],
                [view.bottomAnchor constraintEqualToAnchor:inView.bottomAnchor constant:-padding],
                [view.leadingAnchor constraintEqualToAnchor:inView.leadingAnchor constant:padding],
                [view.trailingAnchor constraintEqualToAnchor:inView.trailingAnchor constant:-padding]
            ]];
        }
        // arrange vertically
        inline void vStack(NSArray* views, UIView* inView, CGFloat spacing = 8) {
            UIView* previousView = nil;
            for (UIView* view in views) {
                view.translatesAutoresizingMaskIntoConstraints = NO;
                [NSLayoutConstraint activateConstraints:@[
                    [view.centerXAnchor constraintEqualToAnchor:inView.centerXAnchor]
                ]];
                if (previousView) {
                    [view.topAnchor constraintEqualToAnchor:previousView.bottomAnchor 
                                                   constant:spacing].active = YES;
                } else {
                    [view.topAnchor constraintEqualToAnchor:inView.topAnchor 
                                                   constant:spacing].active = YES;
                }
                previousView = view;
            }
            if (previousView) {
                [previousView.bottomAnchor constraintLessThanOrEqualToAnchor:inView.bottomAnchor 
                                                                     constant:-spacing].active = YES;
            }
        }
        inline constexpr auto& vstack = vStack;
        // add constraint shortcuts
        inline void width(UIView* view, CGFloat width) { [view.widthAnchor constraintEqualToConstant:width].active = YES; }
        inline void height(UIView* view, CGFloat height) { [view.heightAnchor constraintEqualToConstant:height].active = YES; }
        inline void size(UIView* view, CGFloat width, CGFloat height) {
            [NSLayoutConstraint activateConstraints:@[
                [view.widthAnchor constraintEqualToConstant:width],
                [view.heightAnchor constraintEqualToConstant:height]
            ]];
        }
    } // namespace objc::layout
    namespace objc::alert {
        inline void show(UIViewController* vc, NSString* title, NSString* message) {
            UIAlertController* alert = [UIAlertController alertControllerWithTitle:title
                                                                           message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction* ok = [UIAlertAction actionWithTitle:@"OK"
                                                         style:UIAlertActionStyleDefault
                                                       handler:nil];
            [alert addAction:ok];
            [vc presentViewController:alert animated:YES completion:nil];
        }
        inline void confirm(UIViewController* vc, NSString* title, NSString* message, 
                            void (^completion)(BOOL confirmed)) {
            UIAlertController* alert = [UIAlertController alertControllerWithTitle:title
                                                                           message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction* cancel = [UIAlertAction actionWithTitle:@"Cancel"
                                                             style:UIAlertActionStyleCancel
                                                           handler:^(UIAlertAction* action) {
                if (completion) completion(NO);
            }];
            UIAlertAction* confirm = [UIAlertAction actionWithTitle:@"Certain"
                                                              style:UIAlertActionStyleDefault
                                                            handler:^(UIAlertAction* action) {
                if (completion) completion(YES);
            }];
            [alert addAction:cancel];
            [alert addAction:confirm];
            [vc presentViewController:alert animated:YES completion:nil];
        }
        inline void confirm(UIViewController* vc, NSString* title, NSString* message,
                            NSString* cancelTitle, NSString* confirmTitle,
                            void (^completion)(BOOL confirmed)) {
            UIAlertController* alert = [
                UIAlertController alertControllerWithTitle:title
                message:message
                preferredStyle:UIAlertControllerStyleAlert
        ];
            UIAlertAction* cancel = [
                UIAlertAction actionWithTitle:cancelTitle
                style:UIAlertActionStyleCancel
                handler:^(UIAlertAction* action) {
                    if (completion) completion(NO);
                }
            ];
            UIAlertAction* confirmAction = [
                UIAlertAction actionWithTitle:confirmTitle
                style:UIAlertActionStyleDefault
                handler:^(UIAlertAction* action) {
                    if (completion) completion(YES);
                }
            ];
            [alert addAction:cancel];
            [alert addAction:confirmAction];
            [vc presentViewController:alert animated:YES completion:nil];
        }
    } // namespace objc::alert
    namespace objc::font {
        inline UIFont* system(CGFloat size) { return [UIFont systemFontOfSize:size]; }
        inline UIFont* bold_system(CGFloat size) { return [UIFont boldSystemFontOfSize:size]; }
        inline constexpr auto& boldsys = bold_system;
        inline UIFont* italic_system(CGFloat size) { return [UIFont italicSystemFontOfSize:size]; }
        inline constexpr auto& italicsys = italic_system;
        inline UIFont* monospaced(CGFloat size) { return [UIFont monospacedDigitSystemFontOfSize:size weight:UIFontWeightRegular]; }
    } // namespace objc::font
    namespace objc::image {
        inline UIImage* system(NSString* name) {
            if (@available(iOS 13.0, *)) { return [UIImage systemImageNamed:name]; }
            return nil;
        }
        inline UIImage* named(NSString* name) { return [UIImage imageNamed:name]; }
    } // namespace objc::image
    namespace objc::img = objc::image;
    // MARK: color auxiliary
    namespace objc::color {
        inline UIColor* white() { return UIColor.whiteColor; }
        inline UIColor* black() { return UIColor.blackColor; }
        inline UIColor* red() { return UIColor.redColor; }
        inline UIColor* system_blue() { return UIColor.systemBlueColor; }
        inline UIColor* green() { return UIColor.greenColor; }
        inline UIColor* yellow() { return UIColor.yellowColor; }
        inline UIColor* gray() { return UIColor.grayColor; }
        inline UIColor* system_red() { return UIColor.systemRedColor; }
        inline UIColor* system_green() { return UIColor.systemGreenColor; }
        inline UIColor* system_background() { return UIColor.systemBackgroundColor; }
        inline constexpr auto& sysblue = system_blue;
        inline constexpr auto& sysred = system_red;
        inline constexpr auto& sysgreen = system_green;
        inline constexpr auto& sysbackground = system_background;
        // custom RGB color
        inline UIColor* rgb(CGFloat r, CGFloat g, CGFloat b) { return [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1.0]; }
        inline UIColor* rgba(CGFloat r, CGFloat g, CGFloat b, CGFloat a) { return [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:a]; }
    } // namespace objc::color
#else
#error "only apple platform and use objc/objc++ code, only then can it be used"
#endif // __OBJC__ && __APPLE__
} // namespace xmz
# endif // XMZ_TEAM_IOS_APPLE_OBJC_HPP

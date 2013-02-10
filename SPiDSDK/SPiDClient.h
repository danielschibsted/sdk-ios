//
//  SPiDClient.h
//  SPiDSDK
//
//  Created by Mikael Lindström on 9/11/12.
//  Copyright (c) 2012 Mikael Lindström. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "SPiDUtils.h"

@class SPiDAuthorizationRequest;
@class SPiDResponse;
@class SPiDAccessToken;
@class SPiDRequest;

static NSString *const defaultAPIVersionSPiD = @"2";
static NSString *const AccessTokenKeychainIdentification = @"AccessToken";

// debug print used by SPiDSDK
#ifdef DEBUG
#   define SPiDDebugLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#   define SPiDDebugLog(...)
#endif

/**
 The main SDK class, all interaction with SPiD goes through this class

 `SPiDClient` contains a singleton instance and all calls to SPiD should go through this instance.
 */

@interface SPiDClient : NSObject

///---------------------------------------------------------------------------------------
/// @name Public properties
///---------------------------------------------------------------------------------------

/** Client ID provided by SPiD  */
@property(strong, nonatomic) NSString *clientID;

/** Client ID to be used when generating a one time code

Defaults to clientID
*/
@property(strong, nonatomic) NSString *serverClientID;

/** Client secret provided by SPiD  */
@property(strong, nonatomic) NSString *clientSecret;

/** Signing secret provided by SPiD  */
@property(strong, nonatomic) NSString *sigSecret;

/** App URL schema

 This is used for generating the redirect URI needed for switching back from safari to app
 */
@property(strong, nonatomic) NSString *appURLScheme;

/** Redirect URI

 This is normally generated by the SDK using the `appURLSchema`://SPiD
 */
@property(strong, nonatomic) NSURL *redirectURI; // Note: Defaults to appURLScheme://SPiD/{login|logout|failure}

/** URL to the SPiD server */
@property(strong, nonatomic) NSURL *serverURL;

/** URL to use for authorization to SPiD

 This URL is normally generated using the `serverURL´/auth/login
 */
@property(strong, nonatomic) NSURL *authorizationURL;

@property(strong, nonatomic) NSURL *registrationURL;

@property(strong, nonatomic) NSURL *lostPasswordURL;

/** URL to use for requesting access token from SPiD

 This URL is normally generated using the `serverURL´/oauth/token
 */
@property(strong, nonatomic) NSURL *tokenURL;

/** Sets if access token should be saved in keychain, default value is YES */
@property(nonatomic) BOOL saveToKeychain;

/** The API version that should be used, defaults to 2 */
@property(strong, nonatomic) NSString *apiVersionSPiD;

/** Use mobile web version for SPiD, default YES */
@property(nonatomic) BOOL useMobileWeb;

/** HTML string that will be show when WebView is loading */
@property(strong, nonatomic) NSString *webViewInitialHTML;

@property(strong, nonatomic) SPiDAccessToken *accessToken;
@property(nonatomic, strong) NSMutableArray *waitingRequests;

///---------------------------------------------------------------------------------------
/// @name Public Methods
///---------------------------------------------------------------------------------------

/** Returns the singleton instance of SPiDClient

 @return Returns singleton instance
 */
+ (SPiDClient *)sharedInstance;

/** Configures the `SPiDClient`

 @param clientID The client ID provided by SPiD
 @param clientSecret The client secret provided by SPiD
 @param appURLSchema The url schema for the app (eg spidtest://)
 @param serverURL The url to SPiD
 */
- (void)setClientID:(NSString *)clientID
       clientSecret:(NSString *)clientSecret
       appURLScheme:(NSString *)appURLSchema
          serverURL:(NSURL *)serverURL;

- (UIWebView *)webViewAuthorizationWithCompletionHandler:(void (^)(NSError *))completionHandler;

- (UIWebView *)webViewRegistrationWithCompletionHandler:(void (^)(NSError *))completionHandler;

- (UIWebView *)webViewLostPasswordWithCompletionHandler:(void (^)(NSError *))completionHandler;


/** Handles URL redirects to the app

@param url Input URL
@return Returns YES if URL was handled by `SPiDClient`
*/
- (BOOL)handleOpenURL:(NSURL *)url;

/** Authorizes with SPiD

 This requires that the `SPiDClient` has been configured.
 Redirects to safari to get code and then uses this to obtain a access token. Any exsisting access token will be logged out.
 The access token is then saved to keychain

 @warning `SPiDClient` has to be configured before calling `authorizationRequestWithCompletionHandler`. The receiver must also check if a error was returned to the _completionHandler.
 @param _completionHandler Run after authorization is completed
 */
- (void)browserRedirectAuthorizationWithCompletionHandler:(void (^)(NSError *response))completionHandler;

/** Logout from SPiD

 This requires that the app has obtained a access token.
 Redirects to safari to logout from SPiD and remove cookie.
 Also removes access token from keychain

 @warning `SPiDClient` has to be logged in before this call. The receiver must also check if a error was returned to the _completionHandler.
 @param _completionHandler Run after logout is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)logoutRequestWithCompletionHandler:(void (^)(NSError *response))completionHandler;

/** Soft logout from SPiD

 This requires that the app has obtained a access token.
 Logout from SPiD without redirect to Safari, cookie will not be removed
 Also removes access token from keychain

 @warning `SPiDClient` has to be logged in before this call .The receiver must also check if a error was returned to the _completionHandler.
 @param _completionHandler Run after logout is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */

- (void)softLogoutRequestWithCompletionHandler:(void (^)(NSError *))completionHandler;

/** Refresh access token

 Forces refresh of access token, this is unusally not needed since SPiDSDK will automatically refresh token when needed.
 The access token is then saved to keychain

 @warning `SPiDClient` has to be logged in before this call. The receiver must also check if a error was returned to the _completionHandler.
 @param _completionHandler Run after authorization is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)refreshAccessTokenRequestWithCompletionHandler:(void (^)(NSError *response))completionHandler;

/** Refreshes the access token and then reruns the request

 Note: The SDK enforces a number of maximum retries per request to stop requests from retrying forever

 @param request The request to rerun after a access token has been received
 */
- (void)refreshAccessTokenAndRerunRequest:(SPiDRequest *)request;

/** Runs a GET request against the SPiD server

 Uses the `apiVersionSPiD` property to generate a SPiD API GET request using the provided path

 @param path Path for the request eg _/me_
 @param _completionHandler Runs after request is completed
 @see sharedInstance
 */
- (void)apiGetRequestWithPath:(NSString *)path completionHandler:(void (^)(SPiDResponse *))completionHandler;

/** Runs a POST request against the SPiD server

 Uses the `apiVersionSPiD` property to generate a SPiD API POST request using the provided path

 @param path Path for the request eg _/me_
 @param body Http body to be posted
 @param _completionHandler Runs after request is completed
 @see sharedInstance
 */
- (void)apiPostRequestWithPath:(NSString *)path body:(NSDictionary *)body completionHandler:(void (^)(SPiDResponse *))completionHandler;

/** Checks if the access token has expired

 @return Returns YES if access token has expired
 */
- (BOOL)hasTokenExpired;

/** Returns the time when access token expires

 @return Returns the date when the access token expires
 */
- (NSDate *)tokenExpiresAt;

/** Returns the user ID for the current user

 @return Returns user ID
 */
- (NSString *)currentUserID;

/** Returns YES if `SPiDClient` has a access token and is logged in

@return Returns YES if `SPiDClient` is logged in
*/
- (BOOL)isAuthorized;

- (BOOL)isClientToken;


///---------------------------------------------------------------------------------------
/// @name Request wrappers
///---------------------------------------------------------------------------------------

/** Requests a one time code to be used server side.
*
 @note The code is generated using the server client id and not the applications client id.
 @warning Requires that the user is authorized with SPiD
 @param _completionHandler Run after request is completed
 */

- (void)getOneTimeCodeRequestWithCompletionHandler:(void (^)(SPiDResponse *))completionHandler;

/** Requests the currently logged in user’s object. Note that the user session does not last as long as the access token, therefor the me request should only be used right after the app has received a access token. The user id should then be saved and used with the `getUserRequestWithID:andCompletionHandler`

 For information about the return object see: <http://www.schibstedpayment.no/docs/doku.php?id=wiki:user_api>

 @warning Requires that the user is authorized with SPiD
 @param _completionHandler Run after request is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)getMeRequestWithCompletionHandler:(void (^)(SPiDResponse *response))completionHandler;

/** Requests the userinformation for the specified userID

 For information about the return object see: <http://www.schibstedpayment.no/docs/doku.php?id=wiki:user_api>

 @warning Requires that the user is authorized with SPiD
 @param userID ID for the selected user
 @param _completionHandler Run after request is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)getUserRequestWithID:(NSString *)userID completionHandler:(void (^)(SPiDResponse *response))completionHandler;

/** Requests the userinformation for the current user

 For information about the return object see: <http://www.schibstedpayment.no/docs/doku.php?id=wiki:user_api>

 @warning Requires that the user is authorized with SPiD
 @param _completionHandler Run after request is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)getCurrentUserRequestWithCompletionHandler:(void (^)(SPiDResponse *))completionHandler;

/** Request all login attempts for a specific client

 For information about the return object see: <http://www.schibstedpayment.no/docs/doku.php?id=wiki:login_api>

 @warning Requires that the user is authorized with SPiD
 @param userID The userID that logins should be fetched for
 @param _completionHandler Run after request is completed
 @see authorizationRequestWithCompletionHandler:
 @see isAuthorized
 */
- (void)getUserLoginsRequestWithUserID:(NSString *)userID completionHandler:(void (^)(SPiDResponse *response))completionHandler;

@end

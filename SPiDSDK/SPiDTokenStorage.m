//
//  SPiDTokenStorage.m
//  SPiDSDK
//
//  Created by Daniel Lazarenko on 07/03/2017.
//

#import "SPiDTokenStorage.h"
#import "SPiDTokenStorageUserDefaultsBackend.h"
#import "SPiDTokenStorageKeychainBackend.h"

static NSString *const AccessTokenKeychainIdentification = @"AccessToken";

@interface SPiDTokenStorage ()
{
    NSArray<NSNumber *> *_readBackendTypes;
    NSArray<NSNumber *> *_writeBackendTypes;
    NSDictionary<NSNumber *, id<SPiDTokenStorageBackend>> *_backends;
}

@property (readonly) NSString *identifier;
@property (readonly) NSArray<id<SPiDTokenStorageBackend>> *readBackends;
@property (readonly) NSArray<id<SPiDTokenStorageBackend>> *writeBackends;

@end

@implementation SPiDTokenStorage

+ (id<SPiDTokenStorageBackend>)createBackendOfType:(SPiDTokenStorageBackendType)type
{
    switch (type) {
        case SPiDTokenStorageBackendTypeKeychain:
            return [SPiDTokenStorageKeychainBackend new];
        case SPiDTokenStorageBackendTypeUserDefaults:
            return [SPiDTokenStorageUserDefaultsBackend new];
        default:
            NSAssert(NO, @"Unknown SPiDTokenStorageBackendType %d", type);
    }
}

- (instancetype)initWithReadBackendTypes:(NSArray<NSNumber *> *)readBackendTypes
                       writeBackendTypes:(NSArray<NSNumber *> *)writeBackendTypes
{
    self = [super init];
    if (self == nil) return nil;
    _readBackendTypes = readBackendTypes;
    _writeBackendTypes = writeBackendTypes;

    NSSet<NSNumber *> *allBackendTypes = [[NSSet setWithArray:readBackendTypes] setByAddingObjectsFromArray:writeBackendTypes];
    NSMutableDictionary<NSNumber *, id<SPiDTokenStorageBackend>> *backends = [NSMutableDictionary new];
    for (NSNumber *type in allBackendTypes) {
        backends[type] = [SPiDTokenStorage createBackendOfType:[type unsignedIntegerValue]];
    }
    _backends = backends;

    return self;
}

- (NSString *)identifier
{
    return AccessTokenKeychainIdentification;
}

- (NSArray<id<SPiDTokenStorageBackend>> *)readBackends
{
    NSMutableArray *backends = [NSMutableArray new];
    for (NSNumber *backendType in _readBackendTypes) {
        [backends addObject:_backends[backendType]];
    }
    return backends;
}

- (NSArray<id<SPiDTokenStorageBackend>> *)writeBackends
{
    NSMutableArray *backends = [NSMutableArray new];
    for (NSNumber *backendType in _writeBackendTypes) {
        [backends addObject:_backends[backendType]];
    }
    return backends;
}

- (SPiDAccessToken *)loadAccessTokenAndReplicate
{
    SPiDAccessToken *token = nil;
    NSNumber *tokenBackendType = nil;
    for (NSNumber *backendType in _readBackendTypes) {
        id<SPiDTokenStorageBackend> backend = _backends[backendType];
        token = [backend accessTokenForIdentifier:self.identifier];
        if (token != nil) {
            tokenBackendType = backendType;
            break;
        }
    }

    if (token == nil) {
        return nil;
    }

    // replicate
    NSMutableSet *replicateBackendTypes = [NSMutableSet setWithArray:_writeBackendTypes];
    [replicateBackendTypes removeObject:tokenBackendType];
    for (NSNumber *backendType in replicateBackendTypes) {
        id<SPiDTokenStorageBackend> backend = _backends[backendType];
        [backend storeAccessTokenWithValue:token forIdentifier:self.identifier];
    }

    return token;
}

- (BOOL)storeAccessTokenWithValue:(SPiDAccessToken *)accessToken
{
    BOOL result = YES;
    for (id<SPiDTokenStorageBackend> backend in self.writeBackends) {
        result &= [backend storeAccessTokenWithValue:accessToken forIdentifier:self.identifier];
    }
    return result;
}

- (BOOL)updateAccessTokenWithValue:(SPiDAccessToken *)accessToken
{
    BOOL result = YES;
    for (id<SPiDTokenStorageBackend> backend in self.writeBackends) {
        result &= [backend updateAccessTokenWithValue:accessToken forIdentifier:self.identifier];
    }
    return result;
}

- (void)removeAccessToken
{
    for (id<SPiDTokenStorageBackend> backend in _backends.allValues) {
        [backend removeAccessTokenForIdentifier:self.identifier];
    }
}

@end

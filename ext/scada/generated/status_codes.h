#ifndef SCADA_STATUS_CODES_H
#define SCADA_STATUS_CODES_H

#include <ruby.h>
#include "deps/open62541.h"

VALUE eScadaBase;
VALUE eScadaBadUnexpectedError;
VALUE eScadaBadInternalError;
VALUE eScadaBadOutOfMemory;
VALUE eScadaBadResourceUnavailable;
VALUE eScadaBadCommunicationError;
VALUE eScadaBadEncodingError;
VALUE eScadaBadDecodingError;
VALUE eScadaBadEncodingLimitsExceeded;
VALUE eScadaBadRequestTooLarge;
VALUE eScadaBadResponseTooLarge;
VALUE eScadaBadUnknownResponse;
VALUE eScadaBadTimeout;
VALUE eScadaBadServiceUnsupported;
VALUE eScadaBadShutdown;
VALUE eScadaBadServerNotConnected;
VALUE eScadaBadServerHalted;
VALUE eScadaBadNothingToDo;
VALUE eScadaBadTooManyOperations;
VALUE eScadaBadTooManyMonitoredItems;
VALUE eScadaBadDataTypeIdUnknown;
VALUE eScadaBadCertificateInvalid;
VALUE eScadaBadSecurityChecksFailed;
VALUE eScadaBadCertificatePolicyCheckFailed;
VALUE eScadaBadCertificateTimeInvalid;
VALUE eScadaBadCertificateIssuerTimeInvalid;
VALUE eScadaBadCertificateHostNameInvalid;
VALUE eScadaBadCertificateUriInvalid;
VALUE eScadaBadCertificateUseNotAllowed;
VALUE eScadaBadCertificateIssuerUseNotAllowed;
VALUE eScadaBadCertificateUntrusted;
VALUE eScadaBadCertificateRevocationUnknown;
VALUE eScadaBadCertificateIssuerRevocationUnknown;
VALUE eScadaBadCertificateRevoked;
VALUE eScadaBadCertificateIssuerRevoked;
VALUE eScadaBadCertificateChainIncomplete;
VALUE eScadaBadUserAccessDenied;
VALUE eScadaBadIdentityTokenInvalid;
VALUE eScadaBadIdentityTokenRejected;
VALUE eScadaBadSecureChannelIdInvalid;
VALUE eScadaBadInvalidTimestamp;
VALUE eScadaBadNonceInvalid;
VALUE eScadaBadSessionIdInvalid;
VALUE eScadaBadSessionClosed;
VALUE eScadaBadSessionNotActivated;
VALUE eScadaBadSubscriptionIdInvalid;
VALUE eScadaBadRequestHeaderInvalid;
VALUE eScadaBadTimestampsToReturnInvalid;
VALUE eScadaBadRequestCancelledByClient;
VALUE eScadaBadTooManyArguments;
VALUE eScadaBadLicenseExpired;
VALUE eScadaBadLicenseLimitsExceeded;
VALUE eScadaBadLicenseNotAvailable;
VALUE eScadaGoodSubscriptionTransferred;
VALUE eScadaGoodCompletesAsynchronously;
VALUE eScadaGoodOverload;
VALUE eScadaGoodClamped;
VALUE eScadaBadNoCommunication;
VALUE eScadaBadWaitingForInitialData;
VALUE eScadaBadNodeIdInvalid;
VALUE eScadaBadNodeIdUnknown;
VALUE eScadaBadAttributeIdInvalid;
VALUE eScadaBadIndexRangeInvalid;
VALUE eScadaBadIndexRangeNoData;
VALUE eScadaBadDataEncodingInvalid;
VALUE eScadaBadDataEncodingUnsupported;
VALUE eScadaBadNotReadable;
VALUE eScadaBadNotWritable;
VALUE eScadaBadOutOfRange;
VALUE eScadaBadNotSupported;
VALUE eScadaBadNotFound;
VALUE eScadaBadObjectDeleted;
VALUE eScadaBadNotImplemented;
VALUE eScadaBadMonitoringModeInvalid;
VALUE eScadaBadMonitoredItemIdInvalid;
VALUE eScadaBadMonitoredItemFilterInvalid;
VALUE eScadaBadMonitoredItemFilterUnsupported;
VALUE eScadaBadFilterNotAllowed;
VALUE eScadaBadStructureMissing;
VALUE eScadaBadEventFilterInvalid;
VALUE eScadaBadContentFilterInvalid;
VALUE eScadaBadFilterOperatorInvalid;
VALUE eScadaBadFilterOperatorUnsupported;
VALUE eScadaBadFilterOperandCountMismatch;
VALUE eScadaBadFilterOperandInvalid;
VALUE eScadaBadFilterElementInvalid;
VALUE eScadaBadFilterLiteralInvalid;
VALUE eScadaBadContinuationPointInvalid;
VALUE eScadaBadNoContinuationPoints;
VALUE eScadaBadReferenceTypeIdInvalid;
VALUE eScadaBadBrowseDirectionInvalid;
VALUE eScadaBadNodeNotInView;
VALUE eScadaBadNumericOverflow;
VALUE eScadaBadServerUriInvalid;
VALUE eScadaBadServerNameMissing;
VALUE eScadaBadDiscoveryUrlMissing;
VALUE eScadaBadSemaphoreFileMissing;
VALUE eScadaBadRequestTypeInvalid;
VALUE eScadaBadSecurityModeRejected;
VALUE eScadaBadSecurityPolicyRejected;
VALUE eScadaBadTooManySessions;
VALUE eScadaBadUserSignatureInvalid;
VALUE eScadaBadApplicationSignatureInvalid;
VALUE eScadaBadNoValidCertificates;
VALUE eScadaBadIdentityChangeNotSupported;
VALUE eScadaBadRequestCancelledByRequest;
VALUE eScadaBadParentNodeIdInvalid;
VALUE eScadaBadReferenceNotAllowed;
VALUE eScadaBadNodeIdRejected;
VALUE eScadaBadNodeIdExists;
VALUE eScadaBadNodeClassInvalid;
VALUE eScadaBadBrowseNameInvalid;
VALUE eScadaBadBrowseNameDuplicated;
VALUE eScadaBadNodeAttributesInvalid;
VALUE eScadaBadTypeDefinitionInvalid;
VALUE eScadaBadSourceNodeIdInvalid;
VALUE eScadaBadTargetNodeIdInvalid;
VALUE eScadaBadDuplicateReferenceNotAllowed;
VALUE eScadaBadInvalidSelfReference;
VALUE eScadaBadReferenceLocalOnly;
VALUE eScadaBadNoDeleteRights;
VALUE eScadaUncertainReferenceNotDeleted;
VALUE eScadaBadServerIndexInvalid;
VALUE eScadaBadViewIdUnknown;
VALUE eScadaBadViewTimestampInvalid;
VALUE eScadaBadViewParameterMismatch;
VALUE eScadaBadViewVersionInvalid;
VALUE eScadaUncertainNotAllNodesAvailable;
VALUE eScadaGoodResultsMayBeIncomplete;
VALUE eScadaBadNotTypeDefinition;
VALUE eScadaUncertainReferenceOutOfServer;
VALUE eScadaBadTooManyMatches;
VALUE eScadaBadQueryTooComplex;
VALUE eScadaBadNoMatch;
VALUE eScadaBadMaxAgeInvalid;
VALUE eScadaBadSecurityModeInsufficient;
VALUE eScadaBadHistoryOperationInvalid;
VALUE eScadaBadHistoryOperationUnsupported;
VALUE eScadaBadInvalidTimestampArgument;
VALUE eScadaBadWriteNotSupported;
VALUE eScadaBadTypeMismatch;
VALUE eScadaBadMethodInvalid;
VALUE eScadaBadArgumentsMissing;
VALUE eScadaBadNotExecutable;
VALUE eScadaBadTooManySubscriptions;
VALUE eScadaBadTooManyPublishRequests;
VALUE eScadaBadNoSubscription;
VALUE eScadaBadSequenceNumberUnknown;
VALUE eScadaGoodRetransmissionQueueNotSupported;
VALUE eScadaBadMessageNotAvailable;
VALUE eScadaBadInsufficientClientProfile;
VALUE eScadaBadStateNotActive;
VALUE eScadaBadAlreadyExists;
VALUE eScadaBadTcpServerTooBusy;
VALUE eScadaBadTcpMessageTypeInvalid;
VALUE eScadaBadTcpSecureChannelUnknown;
VALUE eScadaBadTcpMessageTooLarge;
VALUE eScadaBadTcpNotEnoughResources;
VALUE eScadaBadTcpInternalError;
VALUE eScadaBadTcpEndpointUrlInvalid;
VALUE eScadaBadRequestInterrupted;
VALUE eScadaBadRequestTimeout;
VALUE eScadaBadSecureChannelClosed;
VALUE eScadaBadSecureChannelTokenUnknown;
VALUE eScadaBadSequenceNumberInvalid;
VALUE eScadaBadProtocolVersionUnsupported;
VALUE eScadaBadConfigurationError;
VALUE eScadaBadNotConnected;
VALUE eScadaBadDeviceFailure;
VALUE eScadaBadSensorFailure;
VALUE eScadaBadOutOfService;
VALUE eScadaBadDeadbandFilterInvalid;
VALUE eScadaUncertainNoCommunicationLastUsableValue;
VALUE eScadaUncertainLastUsableValue;
VALUE eScadaUncertainSubstituteValue;
VALUE eScadaUncertainInitialValue;
VALUE eScadaUncertainSensorNotAccurate;
VALUE eScadaUncertainEngineeringUnitsExceeded;
VALUE eScadaUncertainSubNormal;
VALUE eScadaGoodLocalOverride;
VALUE eScadaBadRefreshInProgress;
VALUE eScadaBadConditionAlreadyDisabled;
VALUE eScadaBadConditionAlreadyEnabled;
VALUE eScadaBadConditionDisabled;
VALUE eScadaBadEventIdUnknown;
VALUE eScadaBadEventNotAcknowledgeable;
VALUE eScadaBadDialogNotActive;
VALUE eScadaBadDialogResponseInvalid;
VALUE eScadaBadConditionBranchAlreadyAcked;
VALUE eScadaBadConditionBranchAlreadyConfirmed;
VALUE eScadaBadConditionAlreadyShelved;
VALUE eScadaBadConditionNotShelved;
VALUE eScadaBadShelvingTimeOutOfRange;
VALUE eScadaBadNoData;
VALUE eScadaBadBoundNotFound;
VALUE eScadaBadBoundNotSupported;
VALUE eScadaBadDataLost;
VALUE eScadaBadDataUnavailable;
VALUE eScadaBadEntryExists;
VALUE eScadaBadNoEntryExists;
VALUE eScadaBadTimestampNotSupported;
VALUE eScadaGoodEntryInserted;
VALUE eScadaGoodEntryReplaced;
VALUE eScadaUncertainDataSubNormal;
VALUE eScadaGoodNoData;
VALUE eScadaGoodMoreData;
VALUE eScadaBadAggregateListMismatch;
VALUE eScadaBadAggregateNotSupported;
VALUE eScadaBadAggregateInvalidInputs;
VALUE eScadaBadAggregateConfigurationRejected;
VALUE eScadaGoodDataIgnored;
VALUE eScadaBadRequestNotAllowed;
VALUE eScadaBadRequestNotComplete;
VALUE eScadaBadTransactionPending;
VALUE eScadaBadTicketRequired;
VALUE eScadaBadTicketInvalid;
VALUE eScadaGoodEdited;
VALUE eScadaGoodPostActionFailed;
VALUE eScadaUncertainDominantValueChanged;
VALUE eScadaGoodDependentValueChanged;
VALUE eScadaBadDominantValueChanged;
VALUE eScadaUncertainDependentValueChanged;
VALUE eScadaBadDependentValueChanged;
VALUE eScadaGoodEdited_DependentValueChanged;
VALUE eScadaGoodEdited_DominantValueChanged;
VALUE eScadaGoodEdited_DominantValueChanged_DependentValueChanged;
VALUE eScadaBadEdited_OutOfRange;
VALUE eScadaBadInitialValue_OutOfRange;
VALUE eScadaBadOutOfRange_DominantValueChanged;
VALUE eScadaBadEdited_OutOfRange_DominantValueChanged;
VALUE eScadaBadOutOfRange_DominantValueChanged_DependentValueChanged;
VALUE eScadaBadEdited_OutOfRange_DominantValueChanged_DependentValueChanged;
VALUE eScadaGoodCommunicationEvent;
VALUE eScadaGoodShutdownEvent;
VALUE eScadaGoodCallAgain;
VALUE eScadaGoodNonCriticalTimeout;
VALUE eScadaBadInvalidArgument;
VALUE eScadaBadConnectionRejected;
VALUE eScadaBadDisconnect;
VALUE eScadaBadConnectionClosed;
VALUE eScadaBadInvalidState;
VALUE eScadaBadEndOfStream;
VALUE eScadaBadNoDataAvailable;
VALUE eScadaBadWaitingForResponse;
VALUE eScadaBadOperationAbandoned;
VALUE eScadaBadExpectedStreamToBlock;
VALUE eScadaBadWouldBlock;
VALUE eScadaBadSyntaxError;
VALUE eScadaBadMaxConnectionsReached;

void scada_register_errors(VALUE rb_mError) {
    eScadaBase = rb_define_class_under(rb_mError, "Error", rb_eStandardError);
    rb_gc_register_mark_object(eScadaBase);
    eScadaBadUnexpectedError = rb_define_class_under(rb_mError, "BadUnexpectedError", eScadaBase);
    eScadaBadInternalError = rb_define_class_under(rb_mError, "BadInternalError", eScadaBase);
    eScadaBadOutOfMemory = rb_define_class_under(rb_mError, "BadOutOfMemory", eScadaBase);
    eScadaBadResourceUnavailable = rb_define_class_under(rb_mError, "BadResourceUnavailable", eScadaBase);
    eScadaBadCommunicationError = rb_define_class_under(rb_mError, "BadCommunicationError", eScadaBase);
    eScadaBadEncodingError = rb_define_class_under(rb_mError, "BadEncodingError", eScadaBase);
    eScadaBadDecodingError = rb_define_class_under(rb_mError, "BadDecodingError", eScadaBase);
    eScadaBadEncodingLimitsExceeded = rb_define_class_under(rb_mError, "BadEncodingLimitsExceeded", eScadaBase);
    eScadaBadRequestTooLarge = rb_define_class_under(rb_mError, "BadRequestTooLarge", eScadaBase);
    eScadaBadResponseTooLarge = rb_define_class_under(rb_mError, "BadResponseTooLarge", eScadaBase);
    eScadaBadUnknownResponse = rb_define_class_under(rb_mError, "BadUnknownResponse", eScadaBase);
    eScadaBadTimeout = rb_define_class_under(rb_mError, "BadTimeout", eScadaBase);
    eScadaBadServiceUnsupported = rb_define_class_under(rb_mError, "BadServiceUnsupported", eScadaBase);
    eScadaBadShutdown = rb_define_class_under(rb_mError, "BadShutdown", eScadaBase);
    eScadaBadServerNotConnected = rb_define_class_under(rb_mError, "BadServerNotConnected", eScadaBase);
    eScadaBadServerHalted = rb_define_class_under(rb_mError, "BadServerHalted", eScadaBase);
    eScadaBadNothingToDo = rb_define_class_under(rb_mError, "BadNothingToDo", eScadaBase);
    eScadaBadTooManyOperations = rb_define_class_under(rb_mError, "BadTooManyOperations", eScadaBase);
    eScadaBadTooManyMonitoredItems = rb_define_class_under(rb_mError, "BadTooManyMonitoredItems", eScadaBase);
    eScadaBadDataTypeIdUnknown = rb_define_class_under(rb_mError, "BadDataTypeIdUnknown", eScadaBase);
    eScadaBadCertificateInvalid = rb_define_class_under(rb_mError, "BadCertificateInvalid", eScadaBase);
    eScadaBadSecurityChecksFailed = rb_define_class_under(rb_mError, "BadSecurityChecksFailed", eScadaBase);
    eScadaBadCertificatePolicyCheckFailed = rb_define_class_under(rb_mError, "BadCertificatePolicyCheckFailed", eScadaBase);
    eScadaBadCertificateTimeInvalid = rb_define_class_under(rb_mError, "BadCertificateTimeInvalid", eScadaBase);
    eScadaBadCertificateIssuerTimeInvalid = rb_define_class_under(rb_mError, "BadCertificateIssuerTimeInvalid", eScadaBase);
    eScadaBadCertificateHostNameInvalid = rb_define_class_under(rb_mError, "BadCertificateHostNameInvalid", eScadaBase);
    eScadaBadCertificateUriInvalid = rb_define_class_under(rb_mError, "BadCertificateUriInvalid", eScadaBase);
    eScadaBadCertificateUseNotAllowed = rb_define_class_under(rb_mError, "BadCertificateUseNotAllowed", eScadaBase);
    eScadaBadCertificateIssuerUseNotAllowed = rb_define_class_under(rb_mError, "BadCertificateIssuerUseNotAllowed", eScadaBase);
    eScadaBadCertificateUntrusted = rb_define_class_under(rb_mError, "BadCertificateUntrusted", eScadaBase);
    eScadaBadCertificateRevocationUnknown = rb_define_class_under(rb_mError, "BadCertificateRevocationUnknown", eScadaBase);
    eScadaBadCertificateIssuerRevocationUnknown = rb_define_class_under(rb_mError, "BadCertificateIssuerRevocationUnknown", eScadaBase);
    eScadaBadCertificateRevoked = rb_define_class_under(rb_mError, "BadCertificateRevoked", eScadaBase);
    eScadaBadCertificateIssuerRevoked = rb_define_class_under(rb_mError, "BadCertificateIssuerRevoked", eScadaBase);
    eScadaBadCertificateChainIncomplete = rb_define_class_under(rb_mError, "BadCertificateChainIncomplete", eScadaBase);
    eScadaBadUserAccessDenied = rb_define_class_under(rb_mError, "BadUserAccessDenied", eScadaBase);
    eScadaBadIdentityTokenInvalid = rb_define_class_under(rb_mError, "BadIdentityTokenInvalid", eScadaBase);
    eScadaBadIdentityTokenRejected = rb_define_class_under(rb_mError, "BadIdentityTokenRejected", eScadaBase);
    eScadaBadSecureChannelIdInvalid = rb_define_class_under(rb_mError, "BadSecureChannelIdInvalid", eScadaBase);
    eScadaBadInvalidTimestamp = rb_define_class_under(rb_mError, "BadInvalidTimestamp", eScadaBase);
    eScadaBadNonceInvalid = rb_define_class_under(rb_mError, "BadNonceInvalid", eScadaBase);
    eScadaBadSessionIdInvalid = rb_define_class_under(rb_mError, "BadSessionIdInvalid", eScadaBase);
    eScadaBadSessionClosed = rb_define_class_under(rb_mError, "BadSessionClosed", eScadaBase);
    eScadaBadSessionNotActivated = rb_define_class_under(rb_mError, "BadSessionNotActivated", eScadaBase);
    eScadaBadSubscriptionIdInvalid = rb_define_class_under(rb_mError, "BadSubscriptionIdInvalid", eScadaBase);
    eScadaBadRequestHeaderInvalid = rb_define_class_under(rb_mError, "BadRequestHeaderInvalid", eScadaBase);
    eScadaBadTimestampsToReturnInvalid = rb_define_class_under(rb_mError, "BadTimestampsToReturnInvalid", eScadaBase);
    eScadaBadRequestCancelledByClient = rb_define_class_under(rb_mError, "BadRequestCancelledByClient", eScadaBase);
    eScadaBadTooManyArguments = rb_define_class_under(rb_mError, "BadTooManyArguments", eScadaBase);
    eScadaBadLicenseExpired = rb_define_class_under(rb_mError, "BadLicenseExpired", eScadaBase);
    eScadaBadLicenseLimitsExceeded = rb_define_class_under(rb_mError, "BadLicenseLimitsExceeded", eScadaBase);
    eScadaBadLicenseNotAvailable = rb_define_class_under(rb_mError, "BadLicenseNotAvailable", eScadaBase);
    eScadaGoodSubscriptionTransferred = rb_define_class_under(rb_mError, "GoodSubscriptionTransferred", eScadaBase);
    eScadaGoodCompletesAsynchronously = rb_define_class_under(rb_mError, "GoodCompletesAsynchronously", eScadaBase);
    eScadaGoodOverload = rb_define_class_under(rb_mError, "GoodOverload", eScadaBase);
    eScadaGoodClamped = rb_define_class_under(rb_mError, "GoodClamped", eScadaBase);
    eScadaBadNoCommunication = rb_define_class_under(rb_mError, "BadNoCommunication", eScadaBase);
    eScadaBadWaitingForInitialData = rb_define_class_under(rb_mError, "BadWaitingForInitialData", eScadaBase);
    eScadaBadNodeIdInvalid = rb_define_class_under(rb_mError, "BadNodeIdInvalid", eScadaBase);
    eScadaBadNodeIdUnknown = rb_define_class_under(rb_mError, "BadNodeIdUnknown", eScadaBase);
    eScadaBadAttributeIdInvalid = rb_define_class_under(rb_mError, "BadAttributeIdInvalid", eScadaBase);
    eScadaBadIndexRangeInvalid = rb_define_class_under(rb_mError, "BadIndexRangeInvalid", eScadaBase);
    eScadaBadIndexRangeNoData = rb_define_class_under(rb_mError, "BadIndexRangeNoData", eScadaBase);
    eScadaBadDataEncodingInvalid = rb_define_class_under(rb_mError, "BadDataEncodingInvalid", eScadaBase);
    eScadaBadDataEncodingUnsupported = rb_define_class_under(rb_mError, "BadDataEncodingUnsupported", eScadaBase);
    eScadaBadNotReadable = rb_define_class_under(rb_mError, "BadNotReadable", eScadaBase);
    eScadaBadNotWritable = rb_define_class_under(rb_mError, "BadNotWritable", eScadaBase);
    eScadaBadOutOfRange = rb_define_class_under(rb_mError, "BadOutOfRange", eScadaBase);
    eScadaBadNotSupported = rb_define_class_under(rb_mError, "BadNotSupported", eScadaBase);
    eScadaBadNotFound = rb_define_class_under(rb_mError, "BadNotFound", eScadaBase);
    eScadaBadObjectDeleted = rb_define_class_under(rb_mError, "BadObjectDeleted", eScadaBase);
    eScadaBadNotImplemented = rb_define_class_under(rb_mError, "BadNotImplemented", eScadaBase);
    eScadaBadMonitoringModeInvalid = rb_define_class_under(rb_mError, "BadMonitoringModeInvalid", eScadaBase);
    eScadaBadMonitoredItemIdInvalid = rb_define_class_under(rb_mError, "BadMonitoredItemIdInvalid", eScadaBase);
    eScadaBadMonitoredItemFilterInvalid = rb_define_class_under(rb_mError, "BadMonitoredItemFilterInvalid", eScadaBase);
    eScadaBadMonitoredItemFilterUnsupported = rb_define_class_under(rb_mError, "BadMonitoredItemFilterUnsupported", eScadaBase);
    eScadaBadFilterNotAllowed = rb_define_class_under(rb_mError, "BadFilterNotAllowed", eScadaBase);
    eScadaBadStructureMissing = rb_define_class_under(rb_mError, "BadStructureMissing", eScadaBase);
    eScadaBadEventFilterInvalid = rb_define_class_under(rb_mError, "BadEventFilterInvalid", eScadaBase);
    eScadaBadContentFilterInvalid = rb_define_class_under(rb_mError, "BadContentFilterInvalid", eScadaBase);
    eScadaBadFilterOperatorInvalid = rb_define_class_under(rb_mError, "BadFilterOperatorInvalid", eScadaBase);
    eScadaBadFilterOperatorUnsupported = rb_define_class_under(rb_mError, "BadFilterOperatorUnsupported", eScadaBase);
    eScadaBadFilterOperandCountMismatch = rb_define_class_under(rb_mError, "BadFilterOperandCountMismatch", eScadaBase);
    eScadaBadFilterOperandInvalid = rb_define_class_under(rb_mError, "BadFilterOperandInvalid", eScadaBase);
    eScadaBadFilterElementInvalid = rb_define_class_under(rb_mError, "BadFilterElementInvalid", eScadaBase);
    eScadaBadFilterLiteralInvalid = rb_define_class_under(rb_mError, "BadFilterLiteralInvalid", eScadaBase);
    eScadaBadContinuationPointInvalid = rb_define_class_under(rb_mError, "BadContinuationPointInvalid", eScadaBase);
    eScadaBadNoContinuationPoints = rb_define_class_under(rb_mError, "BadNoContinuationPoints", eScadaBase);
    eScadaBadReferenceTypeIdInvalid = rb_define_class_under(rb_mError, "BadReferenceTypeIdInvalid", eScadaBase);
    eScadaBadBrowseDirectionInvalid = rb_define_class_under(rb_mError, "BadBrowseDirectionInvalid", eScadaBase);
    eScadaBadNodeNotInView = rb_define_class_under(rb_mError, "BadNodeNotInView", eScadaBase);
    eScadaBadNumericOverflow = rb_define_class_under(rb_mError, "BadNumericOverflow", eScadaBase);
    eScadaBadServerUriInvalid = rb_define_class_under(rb_mError, "BadServerUriInvalid", eScadaBase);
    eScadaBadServerNameMissing = rb_define_class_under(rb_mError, "BadServerNameMissing", eScadaBase);
    eScadaBadDiscoveryUrlMissing = rb_define_class_under(rb_mError, "BadDiscoveryUrlMissing", eScadaBase);
    eScadaBadSemaphoreFileMissing = rb_define_class_under(rb_mError, "BadSemaphoreFileMissing", eScadaBase);
    eScadaBadRequestTypeInvalid = rb_define_class_under(rb_mError, "BadRequestTypeInvalid", eScadaBase);
    eScadaBadSecurityModeRejected = rb_define_class_under(rb_mError, "BadSecurityModeRejected", eScadaBase);
    eScadaBadSecurityPolicyRejected = rb_define_class_under(rb_mError, "BadSecurityPolicyRejected", eScadaBase);
    eScadaBadTooManySessions = rb_define_class_under(rb_mError, "BadTooManySessions", eScadaBase);
    eScadaBadUserSignatureInvalid = rb_define_class_under(rb_mError, "BadUserSignatureInvalid", eScadaBase);
    eScadaBadApplicationSignatureInvalid = rb_define_class_under(rb_mError, "BadApplicationSignatureInvalid", eScadaBase);
    eScadaBadNoValidCertificates = rb_define_class_under(rb_mError, "BadNoValidCertificates", eScadaBase);
    eScadaBadIdentityChangeNotSupported = rb_define_class_under(rb_mError, "BadIdentityChangeNotSupported", eScadaBase);
    eScadaBadRequestCancelledByRequest = rb_define_class_under(rb_mError, "BadRequestCancelledByRequest", eScadaBase);
    eScadaBadParentNodeIdInvalid = rb_define_class_under(rb_mError, "BadParentNodeIdInvalid", eScadaBase);
    eScadaBadReferenceNotAllowed = rb_define_class_under(rb_mError, "BadReferenceNotAllowed", eScadaBase);
    eScadaBadNodeIdRejected = rb_define_class_under(rb_mError, "BadNodeIdRejected", eScadaBase);
    eScadaBadNodeIdExists = rb_define_class_under(rb_mError, "BadNodeIdExists", eScadaBase);
    eScadaBadNodeClassInvalid = rb_define_class_under(rb_mError, "BadNodeClassInvalid", eScadaBase);
    eScadaBadBrowseNameInvalid = rb_define_class_under(rb_mError, "BadBrowseNameInvalid", eScadaBase);
    eScadaBadBrowseNameDuplicated = rb_define_class_under(rb_mError, "BadBrowseNameDuplicated", eScadaBase);
    eScadaBadNodeAttributesInvalid = rb_define_class_under(rb_mError, "BadNodeAttributesInvalid", eScadaBase);
    eScadaBadTypeDefinitionInvalid = rb_define_class_under(rb_mError, "BadTypeDefinitionInvalid", eScadaBase);
    eScadaBadSourceNodeIdInvalid = rb_define_class_under(rb_mError, "BadSourceNodeIdInvalid", eScadaBase);
    eScadaBadTargetNodeIdInvalid = rb_define_class_under(rb_mError, "BadTargetNodeIdInvalid", eScadaBase);
    eScadaBadDuplicateReferenceNotAllowed = rb_define_class_under(rb_mError, "BadDuplicateReferenceNotAllowed", eScadaBase);
    eScadaBadInvalidSelfReference = rb_define_class_under(rb_mError, "BadInvalidSelfReference", eScadaBase);
    eScadaBadReferenceLocalOnly = rb_define_class_under(rb_mError, "BadReferenceLocalOnly", eScadaBase);
    eScadaBadNoDeleteRights = rb_define_class_under(rb_mError, "BadNoDeleteRights", eScadaBase);
    eScadaUncertainReferenceNotDeleted = rb_define_class_under(rb_mError, "UncertainReferenceNotDeleted", eScadaBase);
    eScadaBadServerIndexInvalid = rb_define_class_under(rb_mError, "BadServerIndexInvalid", eScadaBase);
    eScadaBadViewIdUnknown = rb_define_class_under(rb_mError, "BadViewIdUnknown", eScadaBase);
    eScadaBadViewTimestampInvalid = rb_define_class_under(rb_mError, "BadViewTimestampInvalid", eScadaBase);
    eScadaBadViewParameterMismatch = rb_define_class_under(rb_mError, "BadViewParameterMismatch", eScadaBase);
    eScadaBadViewVersionInvalid = rb_define_class_under(rb_mError, "BadViewVersionInvalid", eScadaBase);
    eScadaUncertainNotAllNodesAvailable = rb_define_class_under(rb_mError, "UncertainNotAllNodesAvailable", eScadaBase);
    eScadaGoodResultsMayBeIncomplete = rb_define_class_under(rb_mError, "GoodResultsMayBeIncomplete", eScadaBase);
    eScadaBadNotTypeDefinition = rb_define_class_under(rb_mError, "BadNotTypeDefinition", eScadaBase);
    eScadaUncertainReferenceOutOfServer = rb_define_class_under(rb_mError, "UncertainReferenceOutOfServer", eScadaBase);
    eScadaBadTooManyMatches = rb_define_class_under(rb_mError, "BadTooManyMatches", eScadaBase);
    eScadaBadQueryTooComplex = rb_define_class_under(rb_mError, "BadQueryTooComplex", eScadaBase);
    eScadaBadNoMatch = rb_define_class_under(rb_mError, "BadNoMatch", eScadaBase);
    eScadaBadMaxAgeInvalid = rb_define_class_under(rb_mError, "BadMaxAgeInvalid", eScadaBase);
    eScadaBadSecurityModeInsufficient = rb_define_class_under(rb_mError, "BadSecurityModeInsufficient", eScadaBase);
    eScadaBadHistoryOperationInvalid = rb_define_class_under(rb_mError, "BadHistoryOperationInvalid", eScadaBase);
    eScadaBadHistoryOperationUnsupported = rb_define_class_under(rb_mError, "BadHistoryOperationUnsupported", eScadaBase);
    eScadaBadInvalidTimestampArgument = rb_define_class_under(rb_mError, "BadInvalidTimestampArgument", eScadaBase);
    eScadaBadWriteNotSupported = rb_define_class_under(rb_mError, "BadWriteNotSupported", eScadaBase);
    eScadaBadTypeMismatch = rb_define_class_under(rb_mError, "BadTypeMismatch", eScadaBase);
    eScadaBadMethodInvalid = rb_define_class_under(rb_mError, "BadMethodInvalid", eScadaBase);
    eScadaBadArgumentsMissing = rb_define_class_under(rb_mError, "BadArgumentsMissing", eScadaBase);
    eScadaBadNotExecutable = rb_define_class_under(rb_mError, "BadNotExecutable", eScadaBase);
    eScadaBadTooManySubscriptions = rb_define_class_under(rb_mError, "BadTooManySubscriptions", eScadaBase);
    eScadaBadTooManyPublishRequests = rb_define_class_under(rb_mError, "BadTooManyPublishRequests", eScadaBase);
    eScadaBadNoSubscription = rb_define_class_under(rb_mError, "BadNoSubscription", eScadaBase);
    eScadaBadSequenceNumberUnknown = rb_define_class_under(rb_mError, "BadSequenceNumberUnknown", eScadaBase);
    eScadaGoodRetransmissionQueueNotSupported = rb_define_class_under(rb_mError, "GoodRetransmissionQueueNotSupported", eScadaBase);
    eScadaBadMessageNotAvailable = rb_define_class_under(rb_mError, "BadMessageNotAvailable", eScadaBase);
    eScadaBadInsufficientClientProfile = rb_define_class_under(rb_mError, "BadInsufficientClientProfile", eScadaBase);
    eScadaBadStateNotActive = rb_define_class_under(rb_mError, "BadStateNotActive", eScadaBase);
    eScadaBadAlreadyExists = rb_define_class_under(rb_mError, "BadAlreadyExists", eScadaBase);
    eScadaBadTcpServerTooBusy = rb_define_class_under(rb_mError, "BadTcpServerTooBusy", eScadaBase);
    eScadaBadTcpMessageTypeInvalid = rb_define_class_under(rb_mError, "BadTcpMessageTypeInvalid", eScadaBase);
    eScadaBadTcpSecureChannelUnknown = rb_define_class_under(rb_mError, "BadTcpSecureChannelUnknown", eScadaBase);
    eScadaBadTcpMessageTooLarge = rb_define_class_under(rb_mError, "BadTcpMessageTooLarge", eScadaBase);
    eScadaBadTcpNotEnoughResources = rb_define_class_under(rb_mError, "BadTcpNotEnoughResources", eScadaBase);
    eScadaBadTcpInternalError = rb_define_class_under(rb_mError, "BadTcpInternalError", eScadaBase);
    eScadaBadTcpEndpointUrlInvalid = rb_define_class_under(rb_mError, "BadTcpEndpointUrlInvalid", eScadaBase);
    eScadaBadRequestInterrupted = rb_define_class_under(rb_mError, "BadRequestInterrupted", eScadaBase);
    eScadaBadRequestTimeout = rb_define_class_under(rb_mError, "BadRequestTimeout", eScadaBase);
    eScadaBadSecureChannelClosed = rb_define_class_under(rb_mError, "BadSecureChannelClosed", eScadaBase);
    eScadaBadSecureChannelTokenUnknown = rb_define_class_under(rb_mError, "BadSecureChannelTokenUnknown", eScadaBase);
    eScadaBadSequenceNumberInvalid = rb_define_class_under(rb_mError, "BadSequenceNumberInvalid", eScadaBase);
    eScadaBadProtocolVersionUnsupported = rb_define_class_under(rb_mError, "BadProtocolVersionUnsupported", eScadaBase);
    eScadaBadConfigurationError = rb_define_class_under(rb_mError, "BadConfigurationError", eScadaBase);
    eScadaBadNotConnected = rb_define_class_under(rb_mError, "BadNotConnected", eScadaBase);
    eScadaBadDeviceFailure = rb_define_class_under(rb_mError, "BadDeviceFailure", eScadaBase);
    eScadaBadSensorFailure = rb_define_class_under(rb_mError, "BadSensorFailure", eScadaBase);
    eScadaBadOutOfService = rb_define_class_under(rb_mError, "BadOutOfService", eScadaBase);
    eScadaBadDeadbandFilterInvalid = rb_define_class_under(rb_mError, "BadDeadbandFilterInvalid", eScadaBase);
    eScadaUncertainNoCommunicationLastUsableValue = rb_define_class_under(rb_mError, "UncertainNoCommunicationLastUsableValue", eScadaBase);
    eScadaUncertainLastUsableValue = rb_define_class_under(rb_mError, "UncertainLastUsableValue", eScadaBase);
    eScadaUncertainSubstituteValue = rb_define_class_under(rb_mError, "UncertainSubstituteValue", eScadaBase);
    eScadaUncertainInitialValue = rb_define_class_under(rb_mError, "UncertainInitialValue", eScadaBase);
    eScadaUncertainSensorNotAccurate = rb_define_class_under(rb_mError, "UncertainSensorNotAccurate", eScadaBase);
    eScadaUncertainEngineeringUnitsExceeded = rb_define_class_under(rb_mError, "UncertainEngineeringUnitsExceeded", eScadaBase);
    eScadaUncertainSubNormal = rb_define_class_under(rb_mError, "UncertainSubNormal", eScadaBase);
    eScadaGoodLocalOverride = rb_define_class_under(rb_mError, "GoodLocalOverride", eScadaBase);
    eScadaBadRefreshInProgress = rb_define_class_under(rb_mError, "BadRefreshInProgress", eScadaBase);
    eScadaBadConditionAlreadyDisabled = rb_define_class_under(rb_mError, "BadConditionAlreadyDisabled", eScadaBase);
    eScadaBadConditionAlreadyEnabled = rb_define_class_under(rb_mError, "BadConditionAlreadyEnabled", eScadaBase);
    eScadaBadConditionDisabled = rb_define_class_under(rb_mError, "BadConditionDisabled", eScadaBase);
    eScadaBadEventIdUnknown = rb_define_class_under(rb_mError, "BadEventIdUnknown", eScadaBase);
    eScadaBadEventNotAcknowledgeable = rb_define_class_under(rb_mError, "BadEventNotAcknowledgeable", eScadaBase);
    eScadaBadDialogNotActive = rb_define_class_under(rb_mError, "BadDialogNotActive", eScadaBase);
    eScadaBadDialogResponseInvalid = rb_define_class_under(rb_mError, "BadDialogResponseInvalid", eScadaBase);
    eScadaBadConditionBranchAlreadyAcked = rb_define_class_under(rb_mError, "BadConditionBranchAlreadyAcked", eScadaBase);
    eScadaBadConditionBranchAlreadyConfirmed = rb_define_class_under(rb_mError, "BadConditionBranchAlreadyConfirmed", eScadaBase);
    eScadaBadConditionAlreadyShelved = rb_define_class_under(rb_mError, "BadConditionAlreadyShelved", eScadaBase);
    eScadaBadConditionNotShelved = rb_define_class_under(rb_mError, "BadConditionNotShelved", eScadaBase);
    eScadaBadShelvingTimeOutOfRange = rb_define_class_under(rb_mError, "BadShelvingTimeOutOfRange", eScadaBase);
    eScadaBadNoData = rb_define_class_under(rb_mError, "BadNoData", eScadaBase);
    eScadaBadBoundNotFound = rb_define_class_under(rb_mError, "BadBoundNotFound", eScadaBase);
    eScadaBadBoundNotSupported = rb_define_class_under(rb_mError, "BadBoundNotSupported", eScadaBase);
    eScadaBadDataLost = rb_define_class_under(rb_mError, "BadDataLost", eScadaBase);
    eScadaBadDataUnavailable = rb_define_class_under(rb_mError, "BadDataUnavailable", eScadaBase);
    eScadaBadEntryExists = rb_define_class_under(rb_mError, "BadEntryExists", eScadaBase);
    eScadaBadNoEntryExists = rb_define_class_under(rb_mError, "BadNoEntryExists", eScadaBase);
    eScadaBadTimestampNotSupported = rb_define_class_under(rb_mError, "BadTimestampNotSupported", eScadaBase);
    eScadaGoodEntryInserted = rb_define_class_under(rb_mError, "GoodEntryInserted", eScadaBase);
    eScadaGoodEntryReplaced = rb_define_class_under(rb_mError, "GoodEntryReplaced", eScadaBase);
    eScadaUncertainDataSubNormal = rb_define_class_under(rb_mError, "UncertainDataSubNormal", eScadaBase);
    eScadaGoodNoData = rb_define_class_under(rb_mError, "GoodNoData", eScadaBase);
    eScadaGoodMoreData = rb_define_class_under(rb_mError, "GoodMoreData", eScadaBase);
    eScadaBadAggregateListMismatch = rb_define_class_under(rb_mError, "BadAggregateListMismatch", eScadaBase);
    eScadaBadAggregateNotSupported = rb_define_class_under(rb_mError, "BadAggregateNotSupported", eScadaBase);
    eScadaBadAggregateInvalidInputs = rb_define_class_under(rb_mError, "BadAggregateInvalidInputs", eScadaBase);
    eScadaBadAggregateConfigurationRejected = rb_define_class_under(rb_mError, "BadAggregateConfigurationRejected", eScadaBase);
    eScadaGoodDataIgnored = rb_define_class_under(rb_mError, "GoodDataIgnored", eScadaBase);
    eScadaBadRequestNotAllowed = rb_define_class_under(rb_mError, "BadRequestNotAllowed", eScadaBase);
    eScadaBadRequestNotComplete = rb_define_class_under(rb_mError, "BadRequestNotComplete", eScadaBase);
    eScadaBadTransactionPending = rb_define_class_under(rb_mError, "BadTransactionPending", eScadaBase);
    eScadaBadTicketRequired = rb_define_class_under(rb_mError, "BadTicketRequired", eScadaBase);
    eScadaBadTicketInvalid = rb_define_class_under(rb_mError, "BadTicketInvalid", eScadaBase);
    eScadaGoodEdited = rb_define_class_under(rb_mError, "GoodEdited", eScadaBase);
    eScadaGoodPostActionFailed = rb_define_class_under(rb_mError, "GoodPostActionFailed", eScadaBase);
    eScadaUncertainDominantValueChanged = rb_define_class_under(rb_mError, "UncertainDominantValueChanged", eScadaBase);
    eScadaGoodDependentValueChanged = rb_define_class_under(rb_mError, "GoodDependentValueChanged", eScadaBase);
    eScadaBadDominantValueChanged = rb_define_class_under(rb_mError, "BadDominantValueChanged", eScadaBase);
    eScadaUncertainDependentValueChanged = rb_define_class_under(rb_mError, "UncertainDependentValueChanged", eScadaBase);
    eScadaBadDependentValueChanged = rb_define_class_under(rb_mError, "BadDependentValueChanged", eScadaBase);
    eScadaGoodEdited_DependentValueChanged = rb_define_class_under(rb_mError, "GoodEdited_DependentValueChanged", eScadaBase);
    eScadaGoodEdited_DominantValueChanged = rb_define_class_under(rb_mError, "GoodEdited_DominantValueChanged", eScadaBase);
    eScadaGoodEdited_DominantValueChanged_DependentValueChanged = rb_define_class_under(rb_mError, "GoodEdited_DominantValueChanged_DependentValueChanged", eScadaBase);
    eScadaBadEdited_OutOfRange = rb_define_class_under(rb_mError, "BadEdited_OutOfRange", eScadaBase);
    eScadaBadInitialValue_OutOfRange = rb_define_class_under(rb_mError, "BadInitialValue_OutOfRange", eScadaBase);
    eScadaBadOutOfRange_DominantValueChanged = rb_define_class_under(rb_mError, "BadOutOfRange_DominantValueChanged", eScadaBase);
    eScadaBadEdited_OutOfRange_DominantValueChanged = rb_define_class_under(rb_mError, "BadEdited_OutOfRange_DominantValueChanged", eScadaBase);
    eScadaBadOutOfRange_DominantValueChanged_DependentValueChanged = rb_define_class_under(rb_mError, "BadOutOfRange_DominantValueChanged_DependentValueChanged", eScadaBase);
    eScadaBadEdited_OutOfRange_DominantValueChanged_DependentValueChanged = rb_define_class_under(rb_mError, "BadEdited_OutOfRange_DominantValueChanged_DependentValueChanged", eScadaBase);
    eScadaGoodCommunicationEvent = rb_define_class_under(rb_mError, "GoodCommunicationEvent", eScadaBase);
    eScadaGoodShutdownEvent = rb_define_class_under(rb_mError, "GoodShutdownEvent", eScadaBase);
    eScadaGoodCallAgain = rb_define_class_under(rb_mError, "GoodCallAgain", eScadaBase);
    eScadaGoodNonCriticalTimeout = rb_define_class_under(rb_mError, "GoodNonCriticalTimeout", eScadaBase);
    eScadaBadInvalidArgument = rb_define_class_under(rb_mError, "BadInvalidArgument", eScadaBase);
    eScadaBadConnectionRejected = rb_define_class_under(rb_mError, "BadConnectionRejected", eScadaBase);
    eScadaBadDisconnect = rb_define_class_under(rb_mError, "BadDisconnect", eScadaBase);
    eScadaBadConnectionClosed = rb_define_class_under(rb_mError, "BadConnectionClosed", eScadaBase);
    eScadaBadInvalidState = rb_define_class_under(rb_mError, "BadInvalidState", eScadaBase);
    eScadaBadEndOfStream = rb_define_class_under(rb_mError, "BadEndOfStream", eScadaBase);
    eScadaBadNoDataAvailable = rb_define_class_under(rb_mError, "BadNoDataAvailable", eScadaBase);
    eScadaBadWaitingForResponse = rb_define_class_under(rb_mError, "BadWaitingForResponse", eScadaBase);
    eScadaBadOperationAbandoned = rb_define_class_under(rb_mError, "BadOperationAbandoned", eScadaBase);
    eScadaBadExpectedStreamToBlock = rb_define_class_under(rb_mError, "BadExpectedStreamToBlock", eScadaBase);
    eScadaBadWouldBlock = rb_define_class_under(rb_mError, "BadWouldBlock", eScadaBase);
    eScadaBadSyntaxError = rb_define_class_under(rb_mError, "BadSyntaxError", eScadaBase);
    eScadaBadMaxConnectionsReached = rb_define_class_under(rb_mError, "BadMaxConnectionsReached", eScadaBase);
}

void scada_check_status(UA_StatusCode code) {
    if (code == UA_STATUSCODE_GOOD) return;
    switch (code) {
        case UA_STATUSCODE_BADUNEXPECTEDERROR:
            rb_raise(eScadaBadUnexpectedError, "%s", "An unexpected error occurred.");
            break;
        case UA_STATUSCODE_BADINTERNALERROR:
            rb_raise(eScadaBadInternalError, "%s", "An internal error occurred as a result of a programming or configuration error.");
            break;
        case UA_STATUSCODE_BADOUTOFMEMORY:
            rb_raise(eScadaBadOutOfMemory, "%s", "Not enough memory to complete the operation.");
            break;
        case UA_STATUSCODE_BADRESOURCEUNAVAILABLE:
            rb_raise(eScadaBadResourceUnavailable, "%s", "An operating system resource is not available.");
            break;
        case UA_STATUSCODE_BADCOMMUNICATIONERROR:
            rb_raise(eScadaBadCommunicationError, "%s", "A low level communication error occurred.");
            break;
        case UA_STATUSCODE_BADENCODINGERROR:
            rb_raise(eScadaBadEncodingError, "%s", "Encoding halted because of invalid data in the objects being serialized.");
            break;
        case UA_STATUSCODE_BADDECODINGERROR:
            rb_raise(eScadaBadDecodingError, "%s", "Decoding halted because of invalid data in the stream.");
            break;
        case UA_STATUSCODE_BADENCODINGLIMITSEXCEEDED:
            rb_raise(eScadaBadEncodingLimitsExceeded, "%s", "The message encoding/decoding limits imposed by the stack have been exceeded.");
            break;
        case UA_STATUSCODE_BADREQUESTTOOLARGE:
            rb_raise(eScadaBadRequestTooLarge, "%s", "The request message size exceeds limits set by the server.");
            break;
        case UA_STATUSCODE_BADRESPONSETOOLARGE:
            rb_raise(eScadaBadResponseTooLarge, "%s", "The response message size exceeds limits set by the client.");
            break;
        case UA_STATUSCODE_BADUNKNOWNRESPONSE:
            rb_raise(eScadaBadUnknownResponse, "%s", "An unrecognized response was received from the server.");
            break;
        case UA_STATUSCODE_BADTIMEOUT:
            rb_raise(eScadaBadTimeout, "%s", "The operation timed out.");
            break;
        case UA_STATUSCODE_BADSERVICEUNSUPPORTED:
            rb_raise(eScadaBadServiceUnsupported, "%s", "The server does not support the requested service.");
            break;
        case UA_STATUSCODE_BADSHUTDOWN:
            rb_raise(eScadaBadShutdown, "%s", "The operation was cancelled because the application is shutting down.");
            break;
        case UA_STATUSCODE_BADSERVERNOTCONNECTED:
            rb_raise(eScadaBadServerNotConnected, "%s", "The operation could not complete because the client is not connected to the server.");
            break;
        case UA_STATUSCODE_BADSERVERHALTED:
            rb_raise(eScadaBadServerHalted, "%s", "The server has stopped and cannot process any requests.");
            break;
        case UA_STATUSCODE_BADNOTHINGTODO:
            rb_raise(eScadaBadNothingToDo, "%s", "There was nothing to do because the client passed a list of operations with no elements.");
            break;
        case UA_STATUSCODE_BADTOOMANYOPERATIONS:
            rb_raise(eScadaBadTooManyOperations, "%s", "The request could not be processed because it specified too many operations.");
            break;
        case UA_STATUSCODE_BADTOOMANYMONITOREDITEMS:
            rb_raise(eScadaBadTooManyMonitoredItems, "%s", "The request could not be processed because there are too many monitored items in the subscription.");
            break;
        case UA_STATUSCODE_BADDATATYPEIDUNKNOWN:
            rb_raise(eScadaBadDataTypeIdUnknown, "%s", "The extension object cannot be (de)serialized because the data type id is not recognized.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEINVALID:
            rb_raise(eScadaBadCertificateInvalid, "%s", "The certificate provided as a parameter is not valid.");
            break;
        case UA_STATUSCODE_BADSECURITYCHECKSFAILED:
            rb_raise(eScadaBadSecurityChecksFailed, "%s", "An error occurred verifying security.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEPOLICYCHECKFAILED:
            rb_raise(eScadaBadCertificatePolicyCheckFailed, "%s", "The certificate does not meet the requirements of the security policy.");
            break;
        case UA_STATUSCODE_BADCERTIFICATETIMEINVALID:
            rb_raise(eScadaBadCertificateTimeInvalid, "%s", "The certificate has expired or is not yet valid.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEISSUERTIMEINVALID:
            rb_raise(eScadaBadCertificateIssuerTimeInvalid, "%s", "An issuer certificate has expired or is not yet valid.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEHOSTNAMEINVALID:
            rb_raise(eScadaBadCertificateHostNameInvalid, "%s", "The HostName used to connect to a server does not match a HostName in the certificate.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEURIINVALID:
            rb_raise(eScadaBadCertificateUriInvalid, "%s", "The URI specified in the ApplicationDescription does not match the URI in the certificate.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEUSENOTALLOWED:
            rb_raise(eScadaBadCertificateUseNotAllowed, "%s", "The certificate may not be used for the requested operation.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEISSUERUSENOTALLOWED:
            rb_raise(eScadaBadCertificateIssuerUseNotAllowed, "%s", "The issuer certificate may not be used for the requested operation.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEUNTRUSTED:
            rb_raise(eScadaBadCertificateUntrusted, "%s", "The certificate is not trusted.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEREVOCATIONUNKNOWN:
            rb_raise(eScadaBadCertificateRevocationUnknown, "%s", "It was not possible to determine if the certificate has been revoked.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEISSUERREVOCATIONUNKNOWN:
            rb_raise(eScadaBadCertificateIssuerRevocationUnknown, "%s", "It was not possible to determine if the issuer certificate has been revoked.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEREVOKED:
            rb_raise(eScadaBadCertificateRevoked, "%s", "The certificate has been revoked.");
            break;
        case UA_STATUSCODE_BADCERTIFICATEISSUERREVOKED:
            rb_raise(eScadaBadCertificateIssuerRevoked, "%s", "The issuer certificate has been revoked.");
            break;
        case UA_STATUSCODE_BADCERTIFICATECHAININCOMPLETE:
            rb_raise(eScadaBadCertificateChainIncomplete, "%s", "The certificate chain is incomplete.");
            break;
        case UA_STATUSCODE_BADUSERACCESSDENIED:
            rb_raise(eScadaBadUserAccessDenied, "%s", "User does not have permission to perform the requested operation.");
            break;
        case UA_STATUSCODE_BADIDENTITYTOKENINVALID:
            rb_raise(eScadaBadIdentityTokenInvalid, "%s", "The user identity token is not valid.");
            break;
        case UA_STATUSCODE_BADIDENTITYTOKENREJECTED:
            rb_raise(eScadaBadIdentityTokenRejected, "%s", "The user identity token is valid but the server has rejected it.");
            break;
        case UA_STATUSCODE_BADSECURECHANNELIDINVALID:
            rb_raise(eScadaBadSecureChannelIdInvalid, "%s", "The specified secure channel is no longer valid.");
            break;
        case UA_STATUSCODE_BADINVALIDTIMESTAMP:
            rb_raise(eScadaBadInvalidTimestamp, "%s", "The timestamp is outside the range allowed by the server.");
            break;
        case UA_STATUSCODE_BADNONCEINVALID:
            rb_raise(eScadaBadNonceInvalid, "%s", "The nonce does appear to be not a random value or it is not the correct length.");
            break;
        case UA_STATUSCODE_BADSESSIONIDINVALID:
            rb_raise(eScadaBadSessionIdInvalid, "%s", "The session id is not valid.");
            break;
        case UA_STATUSCODE_BADSESSIONCLOSED:
            rb_raise(eScadaBadSessionClosed, "%s", "The session was closed by the client.");
            break;
        case UA_STATUSCODE_BADSESSIONNOTACTIVATED:
            rb_raise(eScadaBadSessionNotActivated, "%s", "The session cannot be used because ActivateSession has not been called.");
            break;
        case UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID:
            rb_raise(eScadaBadSubscriptionIdInvalid, "%s", "The subscription id is not valid.");
            break;
        case UA_STATUSCODE_BADREQUESTHEADERINVALID:
            rb_raise(eScadaBadRequestHeaderInvalid, "%s", "The header for the request is missing or invalid.");
            break;
        case UA_STATUSCODE_BADTIMESTAMPSTORETURNINVALID:
            rb_raise(eScadaBadTimestampsToReturnInvalid, "%s", "The timestamps to return parameter is invalid.");
            break;
        case UA_STATUSCODE_BADREQUESTCANCELLEDBYCLIENT:
            rb_raise(eScadaBadRequestCancelledByClient, "%s", "The request was cancelled by the client.");
            break;
        case UA_STATUSCODE_BADTOOMANYARGUMENTS:
            rb_raise(eScadaBadTooManyArguments, "%s", "Too many arguments were provided.");
            break;
        case UA_STATUSCODE_BADLICENSEEXPIRED:
            rb_raise(eScadaBadLicenseExpired, "%s", "The server requires a license to operate in general or to perform a service or operation, but existing license is expired.");
            break;
        case UA_STATUSCODE_BADLICENSELIMITSEXCEEDED:
            rb_raise(eScadaBadLicenseLimitsExceeded, "%s", "The server has limits on number of allowed operations / objects, based on installed licenses, and these limits where exceeded.");
            break;
        case UA_STATUSCODE_BADLICENSENOTAVAILABLE:
            rb_raise(eScadaBadLicenseNotAvailable, "%s", "The server does not have a license which is required to operate in general or to perform a service or operation.");
            break;
        case UA_STATUSCODE_GOODSUBSCRIPTIONTRANSFERRED:
            rb_raise(eScadaGoodSubscriptionTransferred, "%s", "The subscription was transferred to another session.");
            break;
        case UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY:
            rb_raise(eScadaGoodCompletesAsynchronously, "%s", "The processing will complete asynchronously.");
            break;
        case UA_STATUSCODE_GOODOVERLOAD:
            rb_raise(eScadaGoodOverload, "%s", "Sampling has slowed down due to resource limitations.");
            break;
        case UA_STATUSCODE_GOODCLAMPED:
            rb_raise(eScadaGoodClamped, "%s", "The value written was accepted but was clamped.");
            break;
        case UA_STATUSCODE_BADNOCOMMUNICATION:
            rb_raise(eScadaBadNoCommunication, "%s", "Communication with the data source is defined, but not established, and there is no last known value available.");
            break;
        case UA_STATUSCODE_BADWAITINGFORINITIALDATA:
            rb_raise(eScadaBadWaitingForInitialData, "%s", "Waiting for the server to obtain values from the underlying data source.");
            break;
        case UA_STATUSCODE_BADNODEIDINVALID:
            rb_raise(eScadaBadNodeIdInvalid, "%s", "The syntax of the node id is not valid.");
            break;
        case UA_STATUSCODE_BADNODEIDUNKNOWN:
            rb_raise(eScadaBadNodeIdUnknown, "%s", "The node id refers to a node that does not exist in the server address space.");
            break;
        case UA_STATUSCODE_BADATTRIBUTEIDINVALID:
            rb_raise(eScadaBadAttributeIdInvalid, "%s", "The attribute is not supported for the specified Node.");
            break;
        case UA_STATUSCODE_BADINDEXRANGEINVALID:
            rb_raise(eScadaBadIndexRangeInvalid, "%s", "The syntax of the index range parameter is invalid.");
            break;
        case UA_STATUSCODE_BADINDEXRANGENODATA:
            rb_raise(eScadaBadIndexRangeNoData, "%s", "No data exists within the range of indexes specified.");
            break;
        case UA_STATUSCODE_BADDATAENCODINGINVALID:
            rb_raise(eScadaBadDataEncodingInvalid, "%s", "The data encoding is invalid.");
            break;
        case UA_STATUSCODE_BADDATAENCODINGUNSUPPORTED:
            rb_raise(eScadaBadDataEncodingUnsupported, "%s", "The server does not support the requested data encoding for the node.");
            break;
        case UA_STATUSCODE_BADNOTREADABLE:
            rb_raise(eScadaBadNotReadable, "%s", "The access level does not allow reading or subscribing to the Node.");
            break;
        case UA_STATUSCODE_BADNOTWRITABLE:
            rb_raise(eScadaBadNotWritable, "%s", "The access level does not allow writing to the Node.");
            break;
        case UA_STATUSCODE_BADOUTOFRANGE:
            rb_raise(eScadaBadOutOfRange, "%s", "The value was out of range.");
            break;
        case UA_STATUSCODE_BADNOTSUPPORTED:
            rb_raise(eScadaBadNotSupported, "%s", "The requested operation is not supported.");
            break;
        case UA_STATUSCODE_BADNOTFOUND:
            rb_raise(eScadaBadNotFound, "%s", "A requested item was not found or a search operation ended without success.");
            break;
        case UA_STATUSCODE_BADOBJECTDELETED:
            rb_raise(eScadaBadObjectDeleted, "%s", "The object cannot be used because it has been deleted.");
            break;
        case UA_STATUSCODE_BADNOTIMPLEMENTED:
            rb_raise(eScadaBadNotImplemented, "%s", "Requested operation is not implemented.");
            break;
        case UA_STATUSCODE_BADMONITORINGMODEINVALID:
            rb_raise(eScadaBadMonitoringModeInvalid, "%s", "The monitoring mode is invalid.");
            break;
        case UA_STATUSCODE_BADMONITOREDITEMIDINVALID:
            rb_raise(eScadaBadMonitoredItemIdInvalid, "%s", "The monitoring item id does not refer to a valid monitored item.");
            break;
        case UA_STATUSCODE_BADMONITOREDITEMFILTERINVALID:
            rb_raise(eScadaBadMonitoredItemFilterInvalid, "%s", "The monitored item filter parameter is not valid.");
            break;
        case UA_STATUSCODE_BADMONITOREDITEMFILTERUNSUPPORTED:
            rb_raise(eScadaBadMonitoredItemFilterUnsupported, "%s", "The server does not support the requested monitored item filter.");
            break;
        case UA_STATUSCODE_BADFILTERNOTALLOWED:
            rb_raise(eScadaBadFilterNotAllowed, "%s", "A monitoring filter cannot be used in combination with the attribute specified.");
            break;
        case UA_STATUSCODE_BADSTRUCTUREMISSING:
            rb_raise(eScadaBadStructureMissing, "%s", "A mandatory structured parameter was missing or null.");
            break;
        case UA_STATUSCODE_BADEVENTFILTERINVALID:
            rb_raise(eScadaBadEventFilterInvalid, "%s", "The event filter is not valid.");
            break;
        case UA_STATUSCODE_BADCONTENTFILTERINVALID:
            rb_raise(eScadaBadContentFilterInvalid, "%s", "The content filter is not valid.");
            break;
        case UA_STATUSCODE_BADFILTEROPERATORINVALID:
            rb_raise(eScadaBadFilterOperatorInvalid, "%s", "An unrecognized operator was provided in a filter.");
            break;
        case UA_STATUSCODE_BADFILTEROPERATORUNSUPPORTED:
            rb_raise(eScadaBadFilterOperatorUnsupported, "%s", "A valid operator was provided, but the server does not provide support for this filter operator.");
            break;
        case UA_STATUSCODE_BADFILTEROPERANDCOUNTMISMATCH:
            rb_raise(eScadaBadFilterOperandCountMismatch, "%s", "The number of operands provided for the filter operator was less then expected for the operand provided.");
            break;
        case UA_STATUSCODE_BADFILTEROPERANDINVALID:
            rb_raise(eScadaBadFilterOperandInvalid, "%s", "The operand used in a content filter is not valid.");
            break;
        case UA_STATUSCODE_BADFILTERELEMENTINVALID:
            rb_raise(eScadaBadFilterElementInvalid, "%s", "The referenced element is not a valid element in the content filter.");
            break;
        case UA_STATUSCODE_BADFILTERLITERALINVALID:
            rb_raise(eScadaBadFilterLiteralInvalid, "%s", "The referenced literal is not a valid value.");
            break;
        case UA_STATUSCODE_BADCONTINUATIONPOINTINVALID:
            rb_raise(eScadaBadContinuationPointInvalid, "%s", "The continuation point provide is longer valid.");
            break;
        case UA_STATUSCODE_BADNOCONTINUATIONPOINTS:
            rb_raise(eScadaBadNoContinuationPoints, "%s", "The operation could not be processed because all continuation points have been allocated.");
            break;
        case UA_STATUSCODE_BADREFERENCETYPEIDINVALID:
            rb_raise(eScadaBadReferenceTypeIdInvalid, "%s", "The reference type id does not refer to a valid reference type node.");
            break;
        case UA_STATUSCODE_BADBROWSEDIRECTIONINVALID:
            rb_raise(eScadaBadBrowseDirectionInvalid, "%s", "The browse direction is not valid.");
            break;
        case UA_STATUSCODE_BADNODENOTINVIEW:
            rb_raise(eScadaBadNodeNotInView, "%s", "The node is not part of the view.");
            break;
        case UA_STATUSCODE_BADNUMERICOVERFLOW:
            rb_raise(eScadaBadNumericOverflow, "%s", "The number was not accepted because of a numeric overflow.");
            break;
        case UA_STATUSCODE_BADSERVERURIINVALID:
            rb_raise(eScadaBadServerUriInvalid, "%s", "The ServerUri is not a valid URI.");
            break;
        case UA_STATUSCODE_BADSERVERNAMEMISSING:
            rb_raise(eScadaBadServerNameMissing, "%s", "No ServerName was specified.");
            break;
        case UA_STATUSCODE_BADDISCOVERYURLMISSING:
            rb_raise(eScadaBadDiscoveryUrlMissing, "%s", "No DiscoveryUrl was specified.");
            break;
        case UA_STATUSCODE_BADSEMAPHOREFILEMISSING:
            rb_raise(eScadaBadSemaphoreFileMissing, "%s", "The semaphore file specified by the client is not valid.");
            break;
        case UA_STATUSCODE_BADREQUESTTYPEINVALID:
            rb_raise(eScadaBadRequestTypeInvalid, "%s", "The security token request type is not valid.");
            break;
        case UA_STATUSCODE_BADSECURITYMODEREJECTED:
            rb_raise(eScadaBadSecurityModeRejected, "%s", "The security mode does not meet the requirements set by the server.");
            break;
        case UA_STATUSCODE_BADSECURITYPOLICYREJECTED:
            rb_raise(eScadaBadSecurityPolicyRejected, "%s", "The security policy does not meet the requirements set by the server.");
            break;
        case UA_STATUSCODE_BADTOOMANYSESSIONS:
            rb_raise(eScadaBadTooManySessions, "%s", "The server has reached its maximum number of sessions.");
            break;
        case UA_STATUSCODE_BADUSERSIGNATUREINVALID:
            rb_raise(eScadaBadUserSignatureInvalid, "%s", "The user token signature is missing or invalid.");
            break;
        case UA_STATUSCODE_BADAPPLICATIONSIGNATUREINVALID:
            rb_raise(eScadaBadApplicationSignatureInvalid, "%s", "The signature generated with the client certificate is missing or invalid.");
            break;
        case UA_STATUSCODE_BADNOVALIDCERTIFICATES:
            rb_raise(eScadaBadNoValidCertificates, "%s", "The client did not provide at least one software certificate that is valid and meets the profile requirements for the server.");
            break;
        case UA_STATUSCODE_BADIDENTITYCHANGENOTSUPPORTED:
            rb_raise(eScadaBadIdentityChangeNotSupported, "%s", "The server does not support changing the user identity assigned to the session.");
            break;
        case UA_STATUSCODE_BADREQUESTCANCELLEDBYREQUEST:
            rb_raise(eScadaBadRequestCancelledByRequest, "%s", "The request was cancelled by the client with the Cancel service.");
            break;
        case UA_STATUSCODE_BADPARENTNODEIDINVALID:
            rb_raise(eScadaBadParentNodeIdInvalid, "%s", "The parent node id does not to refer to a valid node.");
            break;
        case UA_STATUSCODE_BADREFERENCENOTALLOWED:
            rb_raise(eScadaBadReferenceNotAllowed, "%s", "The reference could not be created because it violates constraints imposed by the data model.");
            break;
        case UA_STATUSCODE_BADNODEIDREJECTED:
            rb_raise(eScadaBadNodeIdRejected, "%s", "The requested node id was reject because it was either invalid or server does not allow node ids to be specified by the client.");
            break;
        case UA_STATUSCODE_BADNODEIDEXISTS:
            rb_raise(eScadaBadNodeIdExists, "%s", "The requested node id is already used by another node.");
            break;
        case UA_STATUSCODE_BADNODECLASSINVALID:
            rb_raise(eScadaBadNodeClassInvalid, "%s", "The node class is not valid.");
            break;
        case UA_STATUSCODE_BADBROWSENAMEINVALID:
            rb_raise(eScadaBadBrowseNameInvalid, "%s", "The browse name is invalid.");
            break;
        case UA_STATUSCODE_BADBROWSENAMEDUPLICATED:
            rb_raise(eScadaBadBrowseNameDuplicated, "%s", "The browse name is not unique among nodes that share the same relationship with the parent.");
            break;
        case UA_STATUSCODE_BADNODEATTRIBUTESINVALID:
            rb_raise(eScadaBadNodeAttributesInvalid, "%s", "The node attributes are not valid for the node class.");
            break;
        case UA_STATUSCODE_BADTYPEDEFINITIONINVALID:
            rb_raise(eScadaBadTypeDefinitionInvalid, "%s", "The type definition node id does not reference an appropriate type node.");
            break;
        case UA_STATUSCODE_BADSOURCENODEIDINVALID:
            rb_raise(eScadaBadSourceNodeIdInvalid, "%s", "The source node id does not reference a valid node.");
            break;
        case UA_STATUSCODE_BADTARGETNODEIDINVALID:
            rb_raise(eScadaBadTargetNodeIdInvalid, "%s", "The target node id does not reference a valid node.");
            break;
        case UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED:
            rb_raise(eScadaBadDuplicateReferenceNotAllowed, "%s", "The reference type between the nodes is already defined.");
            break;
        case UA_STATUSCODE_BADINVALIDSELFREFERENCE:
            rb_raise(eScadaBadInvalidSelfReference, "%s", "The server does not allow this type of self reference on this node.");
            break;
        case UA_STATUSCODE_BADREFERENCELOCALONLY:
            rb_raise(eScadaBadReferenceLocalOnly, "%s", "The reference type is not valid for a reference to a remote server.");
            break;
        case UA_STATUSCODE_BADNODELETERIGHTS:
            rb_raise(eScadaBadNoDeleteRights, "%s", "The server will not allow the node to be deleted.");
            break;
        case UA_STATUSCODE_UNCERTAINREFERENCENOTDELETED:
            rb_raise(eScadaUncertainReferenceNotDeleted, "%s", "The server was not able to delete all target references.");
            break;
        case UA_STATUSCODE_BADSERVERINDEXINVALID:
            rb_raise(eScadaBadServerIndexInvalid, "%s", "The server index is not valid.");
            break;
        case UA_STATUSCODE_BADVIEWIDUNKNOWN:
            rb_raise(eScadaBadViewIdUnknown, "%s", "The view id does not refer to a valid view node.");
            break;
        case UA_STATUSCODE_BADVIEWTIMESTAMPINVALID:
            rb_raise(eScadaBadViewTimestampInvalid, "%s", "The view timestamp is not available or not supported.");
            break;
        case UA_STATUSCODE_BADVIEWPARAMETERMISMATCH:
            rb_raise(eScadaBadViewParameterMismatch, "%s", "The view parameters are not consistent with each other.");
            break;
        case UA_STATUSCODE_BADVIEWVERSIONINVALID:
            rb_raise(eScadaBadViewVersionInvalid, "%s", "The view version is not available or not supported.");
            break;
        case UA_STATUSCODE_UNCERTAINNOTALLNODESAVAILABLE:
            rb_raise(eScadaUncertainNotAllNodesAvailable, "%s", "The list of references may not be complete because the underlying system is not available.");
            break;
        case UA_STATUSCODE_GOODRESULTSMAYBEINCOMPLETE:
            rb_raise(eScadaGoodResultsMayBeIncomplete, "%s", "The server should have followed a reference to a node in a remote server but did not. The result set may be incomplete.");
            break;
        case UA_STATUSCODE_BADNOTTYPEDEFINITION:
            rb_raise(eScadaBadNotTypeDefinition, "%s", "The provided Nodeid was not a type definition nodeid.");
            break;
        case UA_STATUSCODE_UNCERTAINREFERENCEOUTOFSERVER:
            rb_raise(eScadaUncertainReferenceOutOfServer, "%s", "One of the references to follow in the relative path references to a node in the address space in another server.");
            break;
        case UA_STATUSCODE_BADTOOMANYMATCHES:
            rb_raise(eScadaBadTooManyMatches, "%s", "The requested operation has too many matches to return.");
            break;
        case UA_STATUSCODE_BADQUERYTOOCOMPLEX:
            rb_raise(eScadaBadQueryTooComplex, "%s", "The requested operation requires too many resources in the server.");
            break;
        case UA_STATUSCODE_BADNOMATCH:
            rb_raise(eScadaBadNoMatch, "%s", "The requested operation has no match to return.");
            break;
        case UA_STATUSCODE_BADMAXAGEINVALID:
            rb_raise(eScadaBadMaxAgeInvalid, "%s", "The max age parameter is invalid.");
            break;
        case UA_STATUSCODE_BADSECURITYMODEINSUFFICIENT:
            rb_raise(eScadaBadSecurityModeInsufficient, "%s", "The operation is not permitted over the current secure channel.");
            break;
        case UA_STATUSCODE_BADHISTORYOPERATIONINVALID:
            rb_raise(eScadaBadHistoryOperationInvalid, "%s", "The history details parameter is not valid.");
            break;
        case UA_STATUSCODE_BADHISTORYOPERATIONUNSUPPORTED:
            rb_raise(eScadaBadHistoryOperationUnsupported, "%s", "The server does not support the requested operation.");
            break;
        case UA_STATUSCODE_BADINVALIDTIMESTAMPARGUMENT:
            rb_raise(eScadaBadInvalidTimestampArgument, "%s", "The defined timestamp to return was invalid.");
            break;
        case UA_STATUSCODE_BADWRITENOTSUPPORTED:
            rb_raise(eScadaBadWriteNotSupported, "%s", "The server does not support writing the combination of value, status and timestamps provided.");
            break;
        case UA_STATUSCODE_BADTYPEMISMATCH:
            rb_raise(eScadaBadTypeMismatch, "%s", "The value supplied for the attribute is not of the same type as the attribute's value.");
            break;
        case UA_STATUSCODE_BADMETHODINVALID:
            rb_raise(eScadaBadMethodInvalid, "%s", "The method id does not refer to a method for the specified object.");
            break;
        case UA_STATUSCODE_BADARGUMENTSMISSING:
            rb_raise(eScadaBadArgumentsMissing, "%s", "The client did not specify all of the input arguments for the method.");
            break;
        case UA_STATUSCODE_BADNOTEXECUTABLE:
            rb_raise(eScadaBadNotExecutable, "%s", "The executable attribute does not allow the execution of the method.");
            break;
        case UA_STATUSCODE_BADTOOMANYSUBSCRIPTIONS:
            rb_raise(eScadaBadTooManySubscriptions, "%s", "The server has reached its maximum number of subscriptions.");
            break;
        case UA_STATUSCODE_BADTOOMANYPUBLISHREQUESTS:
            rb_raise(eScadaBadTooManyPublishRequests, "%s", "The server has reached the maximum number of queued publish requests.");
            break;
        case UA_STATUSCODE_BADNOSUBSCRIPTION:
            rb_raise(eScadaBadNoSubscription, "%s", "There is no subscription available for this session.");
            break;
        case UA_STATUSCODE_BADSEQUENCENUMBERUNKNOWN:
            rb_raise(eScadaBadSequenceNumberUnknown, "%s", "The sequence number is unknown to the server.");
            break;
        case UA_STATUSCODE_GOODRETRANSMISSIONQUEUENOTSUPPORTED:
            rb_raise(eScadaGoodRetransmissionQueueNotSupported, "%s", "The Server does not support retransmission queue and acknowledgement of sequence numbers is not available.");
            break;
        case UA_STATUSCODE_BADMESSAGENOTAVAILABLE:
            rb_raise(eScadaBadMessageNotAvailable, "%s", "The requested notification message is no longer available.");
            break;
        case UA_STATUSCODE_BADINSUFFICIENTCLIENTPROFILE:
            rb_raise(eScadaBadInsufficientClientProfile, "%s", "The client of the current session does not support one or more Profiles that are necessary for the subscription.");
            break;
        case UA_STATUSCODE_BADSTATENOTACTIVE:
            rb_raise(eScadaBadStateNotActive, "%s", "The sub-state machine is not currently active.");
            break;
        case UA_STATUSCODE_BADALREADYEXISTS:
            rb_raise(eScadaBadAlreadyExists, "%s", "An equivalent rule already exists.");
            break;
        case UA_STATUSCODE_BADTCPSERVERTOOBUSY:
            rb_raise(eScadaBadTcpServerTooBusy, "%s", "The server cannot process the request because it is too busy.");
            break;
        case UA_STATUSCODE_BADTCPMESSAGETYPEINVALID:
            rb_raise(eScadaBadTcpMessageTypeInvalid, "%s", "The type of the message specified in the header invalid.");
            break;
        case UA_STATUSCODE_BADTCPSECURECHANNELUNKNOWN:
            rb_raise(eScadaBadTcpSecureChannelUnknown, "%s", "The SecureChannelId and/or TokenId are not currently in use.");
            break;
        case UA_STATUSCODE_BADTCPMESSAGETOOLARGE:
            rb_raise(eScadaBadTcpMessageTooLarge, "%s", "The size of the message chunk specified in the header is too large.");
            break;
        case UA_STATUSCODE_BADTCPNOTENOUGHRESOURCES:
            rb_raise(eScadaBadTcpNotEnoughResources, "%s", "There are not enough resources to process the request.");
            break;
        case UA_STATUSCODE_BADTCPINTERNALERROR:
            rb_raise(eScadaBadTcpInternalError, "%s", "An internal error occurred.");
            break;
        case UA_STATUSCODE_BADTCPENDPOINTURLINVALID:
            rb_raise(eScadaBadTcpEndpointUrlInvalid, "%s", "The server does not recognize the QueryString specified.");
            break;
        case UA_STATUSCODE_BADREQUESTINTERRUPTED:
            rb_raise(eScadaBadRequestInterrupted, "%s", "The request could not be sent because of a network interruption.");
            break;
        case UA_STATUSCODE_BADREQUESTTIMEOUT:
            rb_raise(eScadaBadRequestTimeout, "%s", "Timeout occurred while processing the request.");
            break;
        case UA_STATUSCODE_BADSECURECHANNELCLOSED:
            rb_raise(eScadaBadSecureChannelClosed, "%s", "The secure channel has been closed.");
            break;
        case UA_STATUSCODE_BADSECURECHANNELTOKENUNKNOWN:
            rb_raise(eScadaBadSecureChannelTokenUnknown, "%s", "The token has expired or is not recognized.");
            break;
        case UA_STATUSCODE_BADSEQUENCENUMBERINVALID:
            rb_raise(eScadaBadSequenceNumberInvalid, "%s", "The sequence number is not valid.");
            break;
        case UA_STATUSCODE_BADPROTOCOLVERSIONUNSUPPORTED:
            rb_raise(eScadaBadProtocolVersionUnsupported, "%s", "The applications do not have compatible protocol versions.");
            break;
        case UA_STATUSCODE_BADCONFIGURATIONERROR:
            rb_raise(eScadaBadConfigurationError, "%s", "There is a problem with the configuration that affects the usefulness of the value.");
            break;
        case UA_STATUSCODE_BADNOTCONNECTED:
            rb_raise(eScadaBadNotConnected, "%s", "The variable should receive its value from another variable, but has never been configured to do so.");
            break;
        case UA_STATUSCODE_BADDEVICEFAILURE:
            rb_raise(eScadaBadDeviceFailure, "%s", "There has been a failure in the device/data source that generates the value that has affected the value.");
            break;
        case UA_STATUSCODE_BADSENSORFAILURE:
            rb_raise(eScadaBadSensorFailure, "%s", "There has been a failure in the sensor from which the value is derived by the device/data source.");
            break;
        case UA_STATUSCODE_BADOUTOFSERVICE:
            rb_raise(eScadaBadOutOfService, "%s", "The source of the data is not operational.");
            break;
        case UA_STATUSCODE_BADDEADBANDFILTERINVALID:
            rb_raise(eScadaBadDeadbandFilterInvalid, "%s", "The deadband filter is not valid.");
            break;
        case UA_STATUSCODE_UNCERTAINNOCOMMUNICATIONLASTUSABLEVALUE:
            rb_raise(eScadaUncertainNoCommunicationLastUsableValue, "%s", "Communication to the data source has failed. The variable value is the last value that had a good quality.");
            break;
        case UA_STATUSCODE_UNCERTAINLASTUSABLEVALUE:
            rb_raise(eScadaUncertainLastUsableValue, "%s", "Whatever was updating this value has stopped doing so.");
            break;
        case UA_STATUSCODE_UNCERTAINSUBSTITUTEVALUE:
            rb_raise(eScadaUncertainSubstituteValue, "%s", "The value is an operational value that was manually overwritten.");
            break;
        case UA_STATUSCODE_UNCERTAININITIALVALUE:
            rb_raise(eScadaUncertainInitialValue, "%s", "The value is an initial value for a variable that normally receives its value from another variable.");
            break;
        case UA_STATUSCODE_UNCERTAINSENSORNOTACCURATE:
            rb_raise(eScadaUncertainSensorNotAccurate, "%s", "The value is at one of the sensor limits.");
            break;
        case UA_STATUSCODE_UNCERTAINENGINEERINGUNITSEXCEEDED:
            rb_raise(eScadaUncertainEngineeringUnitsExceeded, "%s", "The value is outside of the range of values defined for this parameter.");
            break;
        case UA_STATUSCODE_UNCERTAINSUBNORMAL:
            rb_raise(eScadaUncertainSubNormal, "%s", "The value is derived from multiple sources and has less than the required number of Good sources.");
            break;
        case UA_STATUSCODE_GOODLOCALOVERRIDE:
            rb_raise(eScadaGoodLocalOverride, "%s", "The value has been overridden.");
            break;
        case UA_STATUSCODE_BADREFRESHINPROGRESS:
            rb_raise(eScadaBadRefreshInProgress, "%s", "This Condition refresh failed, a Condition refresh operation is already in progress.");
            break;
        case UA_STATUSCODE_BADCONDITIONALREADYDISABLED:
            rb_raise(eScadaBadConditionAlreadyDisabled, "%s", "This condition has already been disabled.");
            break;
        case UA_STATUSCODE_BADCONDITIONALREADYENABLED:
            rb_raise(eScadaBadConditionAlreadyEnabled, "%s", "This condition has already been enabled.");
            break;
        case UA_STATUSCODE_BADCONDITIONDISABLED:
            rb_raise(eScadaBadConditionDisabled, "%s", "Property not available, this condition is disabled.");
            break;
        case UA_STATUSCODE_BADEVENTIDUNKNOWN:
            rb_raise(eScadaBadEventIdUnknown, "%s", "The specified event id is not recognized.");
            break;
        case UA_STATUSCODE_BADEVENTNOTACKNOWLEDGEABLE:
            rb_raise(eScadaBadEventNotAcknowledgeable, "%s", "The event cannot be acknowledged.");
            break;
        case UA_STATUSCODE_BADDIALOGNOTACTIVE:
            rb_raise(eScadaBadDialogNotActive, "%s", "The dialog condition is not active.");
            break;
        case UA_STATUSCODE_BADDIALOGRESPONSEINVALID:
            rb_raise(eScadaBadDialogResponseInvalid, "%s", "The response is not valid for the dialog.");
            break;
        case UA_STATUSCODE_BADCONDITIONBRANCHALREADYACKED:
            rb_raise(eScadaBadConditionBranchAlreadyAcked, "%s", "The condition branch has already been acknowledged.");
            break;
        case UA_STATUSCODE_BADCONDITIONBRANCHALREADYCONFIRMED:
            rb_raise(eScadaBadConditionBranchAlreadyConfirmed, "%s", "The condition branch has already been confirmed.");
            break;
        case UA_STATUSCODE_BADCONDITIONALREADYSHELVED:
            rb_raise(eScadaBadConditionAlreadyShelved, "%s", "The condition has already been shelved.");
            break;
        case UA_STATUSCODE_BADCONDITIONNOTSHELVED:
            rb_raise(eScadaBadConditionNotShelved, "%s", "The condition is not currently shelved.");
            break;
        case UA_STATUSCODE_BADSHELVINGTIMEOUTOFRANGE:
            rb_raise(eScadaBadShelvingTimeOutOfRange, "%s", "The shelving time not within an acceptable range.");
            break;
        case UA_STATUSCODE_BADNODATA:
            rb_raise(eScadaBadNoData, "%s", "No data exists for the requested time range or event filter.");
            break;
        case UA_STATUSCODE_BADBOUNDNOTFOUND:
            rb_raise(eScadaBadBoundNotFound, "%s", "No data found to provide upper or lower bound value.");
            break;
        case UA_STATUSCODE_BADBOUNDNOTSUPPORTED:
            rb_raise(eScadaBadBoundNotSupported, "%s", "The server cannot retrieve a bound for the variable.");
            break;
        case UA_STATUSCODE_BADDATALOST:
            rb_raise(eScadaBadDataLost, "%s", "Data is missing due to collection started/stopped/lost.");
            break;
        case UA_STATUSCODE_BADDATAUNAVAILABLE:
            rb_raise(eScadaBadDataUnavailable, "%s", "Expected data is unavailable for the requested time range due to an un-mounted volume, an off-line archive or tape, or similar reason for temporary unavailability.");
            break;
        case UA_STATUSCODE_BADENTRYEXISTS:
            rb_raise(eScadaBadEntryExists, "%s", "The data or event was not successfully inserted because a matching entry exists.");
            break;
        case UA_STATUSCODE_BADNOENTRYEXISTS:
            rb_raise(eScadaBadNoEntryExists, "%s", "The data or event was not successfully updated because no matching entry exists.");
            break;
        case UA_STATUSCODE_BADTIMESTAMPNOTSUPPORTED:
            rb_raise(eScadaBadTimestampNotSupported, "%s", "The client requested history using a timestamp format the server does not support (i.e requested ServerTimestamp when server only supports SourceTimestamp).");
            break;
        case UA_STATUSCODE_GOODENTRYINSERTED:
            rb_raise(eScadaGoodEntryInserted, "%s", "The data or event was successfully inserted into the historical database.");
            break;
        case UA_STATUSCODE_GOODENTRYREPLACED:
            rb_raise(eScadaGoodEntryReplaced, "%s", "The data or event field was successfully replaced in the historical database.");
            break;
        case UA_STATUSCODE_UNCERTAINDATASUBNORMAL:
            rb_raise(eScadaUncertainDataSubNormal, "%s", "The value is derived from multiple values and has less than the required number of Good values.");
            break;
        case UA_STATUSCODE_GOODNODATA:
            rb_raise(eScadaGoodNoData, "%s", "No data exists for the requested time range or event filter.");
            break;
        case UA_STATUSCODE_GOODMOREDATA:
            rb_raise(eScadaGoodMoreData, "%s", "The data or event field was successfully replaced in the historical database.");
            break;
        case UA_STATUSCODE_BADAGGREGATELISTMISMATCH:
            rb_raise(eScadaBadAggregateListMismatch, "%s", "The requested number of Aggregates does not match the requested number of NodeIds.");
            break;
        case UA_STATUSCODE_BADAGGREGATENOTSUPPORTED:
            rb_raise(eScadaBadAggregateNotSupported, "%s", "The requested Aggregate is not support by the server.");
            break;
        case UA_STATUSCODE_BADAGGREGATEINVALIDINPUTS:
            rb_raise(eScadaBadAggregateInvalidInputs, "%s", "The aggregate value could not be derived due to invalid data inputs.");
            break;
        case UA_STATUSCODE_BADAGGREGATECONFIGURATIONREJECTED:
            rb_raise(eScadaBadAggregateConfigurationRejected, "%s", "The aggregate configuration is not valid for specified node.");
            break;
        case UA_STATUSCODE_GOODDATAIGNORED:
            rb_raise(eScadaGoodDataIgnored, "%s", "The request specifies fields which are not valid for the EventType or cannot be saved by the historian.");
            break;
        case UA_STATUSCODE_BADREQUESTNOTALLOWED:
            rb_raise(eScadaBadRequestNotAllowed, "%s", "The request was rejected by the server because it did not meet the criteria set by the server.");
            break;
        case UA_STATUSCODE_BADREQUESTNOTCOMPLETE:
            rb_raise(eScadaBadRequestNotComplete, "%s", "The request has not been processed by the server yet.");
            break;
        case UA_STATUSCODE_BADTRANSACTIONPENDING:
            rb_raise(eScadaBadTransactionPending, "%s", "The operation is not allowed because a transaction is in progress.");
            break;
        case UA_STATUSCODE_BADTICKETREQUIRED:
            rb_raise(eScadaBadTicketRequired, "%s", "The device identity needs a ticket before it can be accepted.");
            break;
        case UA_STATUSCODE_BADTICKETINVALID:
            rb_raise(eScadaBadTicketInvalid, "%s", "The device identity needs a ticket before it can be accepted.");
            break;
        case UA_STATUSCODE_GOODEDITED:
            rb_raise(eScadaGoodEdited, "%s", "The value does not come from the real source and has been edited by the server.");
            break;
        case UA_STATUSCODE_GOODPOSTACTIONFAILED:
            rb_raise(eScadaGoodPostActionFailed, "%s", "There was an error in execution of these post-actions.");
            break;
        case UA_STATUSCODE_UNCERTAINDOMINANTVALUECHANGED:
            rb_raise(eScadaUncertainDominantValueChanged, "%s", "The related EngineeringUnit has been changed but the Variable Value is still provided based on the previous unit.");
            break;
        case UA_STATUSCODE_GOODDEPENDENTVALUECHANGED:
            rb_raise(eScadaGoodDependentValueChanged, "%s", "A dependent value has been changed but the change has not been applied to the device.");
            break;
        case UA_STATUSCODE_BADDOMINANTVALUECHANGED:
            rb_raise(eScadaBadDominantValueChanged, "%s", "The related EngineeringUnit has been changed but this change has not been applied to the device. The Variable Value is still dependent on the previous unit but its status is currently Bad.");
            break;
        case UA_STATUSCODE_UNCERTAINDEPENDENTVALUECHANGED:
            rb_raise(eScadaUncertainDependentValueChanged, "%s", "A dependent value has been changed but the change has not been applied to the device. The quality of the dominant variable is uncertain.");
            break;
        case UA_STATUSCODE_BADDEPENDENTVALUECHANGED:
            rb_raise(eScadaBadDependentValueChanged, "%s", "A dependent value has been changed but the change has not been applied to the device. The quality of the dominant variable is Bad.");
            break;
        case UA_STATUSCODE_GOODEDITED_DEPENDENTVALUECHANGED:
            rb_raise(eScadaGoodEdited_DependentValueChanged, "%s", "It is delivered with a dominant Variable value when a dependent Variable has changed but the change has not been applied.");
            break;
        case UA_STATUSCODE_GOODEDITED_DOMINANTVALUECHANGED:
            rb_raise(eScadaGoodEdited_DominantValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant Variable has changed but the change has not been applied.");
            break;
        case UA_STATUSCODE_GOODEDITED_DOMINANTVALUECHANGED_DEPENDENTVALUECHANGED:
            rb_raise(eScadaGoodEdited_DominantValueChanged_DependentValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant or dependent Variable has changed but change has not been applied.");
            break;
        case UA_STATUSCODE_BADEDITED_OUTOFRANGE:
            rb_raise(eScadaBadEdited_OutOfRange, "%s", "It is delivered with a Variable value when Variable has changed but the value is not legal.");
            break;
        case UA_STATUSCODE_BADINITIALVALUE_OUTOFRANGE:
            rb_raise(eScadaBadInitialValue_OutOfRange, "%s", "It is delivered with a Variable value when a source Variable has changed but the value is not legal.");
            break;
        case UA_STATUSCODE_BADOUTOFRANGE_DOMINANTVALUECHANGED:
            rb_raise(eScadaBadOutOfRange_DominantValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant Variable has changed and the value is not legal.");
            break;
        case UA_STATUSCODE_BADEDITED_OUTOFRANGE_DOMINANTVALUECHANGED:
            rb_raise(eScadaBadEdited_OutOfRange_DominantValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant Variable has changed, the value is not legal and the change has not been applied.");
            break;
        case UA_STATUSCODE_BADOUTOFRANGE_DOMINANTVALUECHANGED_DEPENDENTVALUECHANGED:
            rb_raise(eScadaBadOutOfRange_DominantValueChanged_DependentValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant or dependent Variable has changed and the value is not legal.");
            break;
        case UA_STATUSCODE_BADEDITED_OUTOFRANGE_DOMINANTVALUECHANGED_DEPENDENTVALUECHANGED:
            rb_raise(eScadaBadEdited_OutOfRange_DominantValueChanged_DependentValueChanged, "%s", "It is delivered with a dependent Variable value when a dominant or dependent Variable has changed, the value is not legal and the change has not been applied.");
            break;
        case UA_STATUSCODE_GOODCOMMUNICATIONEVENT:
            rb_raise(eScadaGoodCommunicationEvent, "%s", "The communication layer has raised an event.");
            break;
        case UA_STATUSCODE_GOODSHUTDOWNEVENT:
            rb_raise(eScadaGoodShutdownEvent, "%s", "The system is shutting down.");
            break;
        case UA_STATUSCODE_GOODCALLAGAIN:
            rb_raise(eScadaGoodCallAgain, "%s", "The operation is not finished and needs to be called again.");
            break;
        case UA_STATUSCODE_GOODNONCRITICALTIMEOUT:
            rb_raise(eScadaGoodNonCriticalTimeout, "%s", "A non-critical timeout occurred.");
            break;
        case UA_STATUSCODE_BADINVALIDARGUMENT:
            rb_raise(eScadaBadInvalidArgument, "%s", "One or more arguments are invalid.");
            break;
        case UA_STATUSCODE_BADCONNECTIONREJECTED:
            rb_raise(eScadaBadConnectionRejected, "%s", "Could not establish a network connection to remote server.");
            break;
        case UA_STATUSCODE_BADDISCONNECT:
            rb_raise(eScadaBadDisconnect, "%s", "The server has disconnected from the client.");
            break;
        case UA_STATUSCODE_BADCONNECTIONCLOSED:
            rb_raise(eScadaBadConnectionClosed, "%s", "The network connection has been closed.");
            break;
        case UA_STATUSCODE_BADINVALIDSTATE:
            rb_raise(eScadaBadInvalidState, "%s", "The operation cannot be completed because the object is closed, uninitialized or in some other invalid state.");
            break;
        case UA_STATUSCODE_BADENDOFSTREAM:
            rb_raise(eScadaBadEndOfStream, "%s", "Cannot move beyond end of the stream.");
            break;
        case UA_STATUSCODE_BADNODATAAVAILABLE:
            rb_raise(eScadaBadNoDataAvailable, "%s", "No data is currently available for reading from a non-blocking stream.");
            break;
        case UA_STATUSCODE_BADWAITINGFORRESPONSE:
            rb_raise(eScadaBadWaitingForResponse, "%s", "The asynchronous operation is waiting for a response.");
            break;
        case UA_STATUSCODE_BADOPERATIONABANDONED:
            rb_raise(eScadaBadOperationAbandoned, "%s", "The asynchronous operation was abandoned by the caller.");
            break;
        case UA_STATUSCODE_BADEXPECTEDSTREAMTOBLOCK:
            rb_raise(eScadaBadExpectedStreamToBlock, "%s", "The stream did not return all data requested (possibly because it is a non-blocking stream).");
            break;
        case UA_STATUSCODE_BADWOULDBLOCK:
            rb_raise(eScadaBadWouldBlock, "%s", "Non blocking behaviour is required and the operation would block.");
            break;
        case UA_STATUSCODE_BADSYNTAXERROR:
            rb_raise(eScadaBadSyntaxError, "%s", "A value had an invalid syntax.");
            break;
        case UA_STATUSCODE_BADMAXCONNECTIONSREACHED:
            rb_raise(eScadaBadMaxConnectionsReached, "%s", "The operation could not be finished because all available connections are in use.");
            break;
        default:
            rb_raise(eScadaBase, "OPC UA error: 0x%08x", code);
    }
}

#endif /* SCADA_STATUS_CODES_H */

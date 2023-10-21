"""
@generated by mypy-protobuf.  Do not edit manually!
isort:skip_file
"""
import builtins
import google.protobuf.descriptor
import google.protobuf.internal.containers
import google.protobuf.message
import gen_proto.sdv.databroker.v1.types_pb2
import typing
import typing_extensions

DESCRIPTOR: google.protobuf.descriptor.FileDescriptor

class UpdateDatapointsRequest(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    class DatapointsEntry(google.protobuf.message.Message):
        DESCRIPTOR: google.protobuf.descriptor.Descriptor
        KEY_FIELD_NUMBER: builtins.int
        VALUE_FIELD_NUMBER: builtins.int
        key: builtins.int
        @property
        def value(self) -> sdv.databroker.v1.types_pb2.Datapoint: ...
        def __init__(self,
            *,
            key: builtins.int = ...,
            value: typing.Optional[sdv.databroker.v1.types_pb2.Datapoint] = ...,
            ) -> None: ...
        def HasField(self, field_name: typing_extensions.Literal["value",b"value"]) -> builtins.bool: ...
        def ClearField(self, field_name: typing_extensions.Literal["key",b"key","value",b"value"]) -> None: ...

    DATAPOINTS_FIELD_NUMBER: builtins.int
    @property
    def datapoints(self) -> google.protobuf.internal.containers.MessageMap[builtins.int, sdv.databroker.v1.types_pb2.Datapoint]: ...
    def __init__(self,
        *,
        datapoints: typing.Optional[typing.Mapping[builtins.int, sdv.databroker.v1.types_pb2.Datapoint]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["datapoints",b"datapoints"]) -> None: ...
global___UpdateDatapointsRequest = UpdateDatapointsRequest

class UpdateDatapointsReply(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    class ErrorsEntry(google.protobuf.message.Message):
        DESCRIPTOR: google.protobuf.descriptor.Descriptor
        KEY_FIELD_NUMBER: builtins.int
        VALUE_FIELD_NUMBER: builtins.int
        key: builtins.int
        value: sdv.databroker.v1.types_pb2.DatapointError.ValueType
        def __init__(self,
            *,
            key: builtins.int = ...,
            value: sdv.databroker.v1.types_pb2.DatapointError.ValueType = ...,
            ) -> None: ...
        def ClearField(self, field_name: typing_extensions.Literal["key",b"key","value",b"value"]) -> None: ...

    ERRORS_FIELD_NUMBER: builtins.int
    @property
    def errors(self) -> google.protobuf.internal.containers.ScalarMap[builtins.int, sdv.databroker.v1.types_pb2.DatapointError.ValueType]:
        """If empty, everything went well"""
        pass
    def __init__(self,
        *,
        errors: typing.Optional[typing.Mapping[builtins.int, sdv.databroker.v1.types_pb2.DatapointError.ValueType]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["errors",b"errors"]) -> None: ...
global___UpdateDatapointsReply = UpdateDatapointsReply

class StreamDatapointsRequest(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    class DatapointsEntry(google.protobuf.message.Message):
        DESCRIPTOR: google.protobuf.descriptor.Descriptor
        KEY_FIELD_NUMBER: builtins.int
        VALUE_FIELD_NUMBER: builtins.int
        key: builtins.int
        @property
        def value(self) -> sdv.databroker.v1.types_pb2.Datapoint: ...
        def __init__(self,
            *,
            key: builtins.int = ...,
            value: typing.Optional[sdv.databroker.v1.types_pb2.Datapoint] = ...,
            ) -> None: ...
        def HasField(self, field_name: typing_extensions.Literal["value",b"value"]) -> builtins.bool: ...
        def ClearField(self, field_name: typing_extensions.Literal["key",b"key","value",b"value"]) -> None: ...

    DATAPOINTS_FIELD_NUMBER: builtins.int
    @property
    def datapoints(self) -> google.protobuf.internal.containers.MessageMap[builtins.int, sdv.databroker.v1.types_pb2.Datapoint]: ...
    def __init__(self,
        *,
        datapoints: typing.Optional[typing.Mapping[builtins.int, sdv.databroker.v1.types_pb2.Datapoint]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["datapoints",b"datapoints"]) -> None: ...
global___StreamDatapointsRequest = StreamDatapointsRequest

class StreamDatapointsReply(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    class ErrorsEntry(google.protobuf.message.Message):
        DESCRIPTOR: google.protobuf.descriptor.Descriptor
        KEY_FIELD_NUMBER: builtins.int
        VALUE_FIELD_NUMBER: builtins.int
        key: builtins.int
        value: sdv.databroker.v1.types_pb2.DatapointError.ValueType
        def __init__(self,
            *,
            key: builtins.int = ...,
            value: sdv.databroker.v1.types_pb2.DatapointError.ValueType = ...,
            ) -> None: ...
        def ClearField(self, field_name: typing_extensions.Literal["key",b"key","value",b"value"]) -> None: ...

    ERRORS_FIELD_NUMBER: builtins.int
    @property
    def errors(self) -> google.protobuf.internal.containers.ScalarMap[builtins.int, sdv.databroker.v1.types_pb2.DatapointError.ValueType]:
        """If empty, everything went well"""
        pass
    def __init__(self,
        *,
        errors: typing.Optional[typing.Mapping[builtins.int, sdv.databroker.v1.types_pb2.DatapointError.ValueType]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["errors",b"errors"]) -> None: ...
global___StreamDatapointsReply = StreamDatapointsReply

class RegisterDatapointsRequest(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    LIST_FIELD_NUMBER: builtins.int
    @property
    def list(self) -> google.protobuf.internal.containers.RepeatedCompositeFieldContainer[global___RegistrationMetadata]: ...
    def __init__(self,
        *,
        list: typing.Optional[typing.Iterable[global___RegistrationMetadata]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["list",b"list"]) -> None: ...
global___RegisterDatapointsRequest = RegisterDatapointsRequest

class RegistrationMetadata(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    NAME_FIELD_NUMBER: builtins.int
    DATA_TYPE_FIELD_NUMBER: builtins.int
    DESCRIPTION_FIELD_NUMBER: builtins.int
    CHANGE_TYPE_FIELD_NUMBER: builtins.int
    name: typing.Text
    """Name of the data point
    (e.g. "Vehicle.Cabin.Seat.Row1.Pos1.Position" or "Vehicle.Speed")
    """

    data_type: sdv.databroker.v1.types_pb2.DataType.ValueType
    description: typing.Text
    change_type: sdv.databroker.v1.types_pb2.ChangeType.ValueType
    def __init__(self,
        *,
        name: typing.Text = ...,
        data_type: sdv.databroker.v1.types_pb2.DataType.ValueType = ...,
        description: typing.Text = ...,
        change_type: sdv.databroker.v1.types_pb2.ChangeType.ValueType = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["change_type",b"change_type","data_type",b"data_type","description",b"description","name",b"name"]) -> None: ...
global___RegistrationMetadata = RegistrationMetadata

class RegisterDatapointsReply(google.protobuf.message.Message):
    DESCRIPTOR: google.protobuf.descriptor.Descriptor
    class ResultsEntry(google.protobuf.message.Message):
        DESCRIPTOR: google.protobuf.descriptor.Descriptor
        KEY_FIELD_NUMBER: builtins.int
        VALUE_FIELD_NUMBER: builtins.int
        key: typing.Text
        value: builtins.int
        def __init__(self,
            *,
            key: typing.Text = ...,
            value: builtins.int = ...,
            ) -> None: ...
        def ClearField(self, field_name: typing_extensions.Literal["key",b"key","value",b"value"]) -> None: ...

    RESULTS_FIELD_NUMBER: builtins.int
    @property
    def results(self) -> google.protobuf.internal.containers.ScalarMap[typing.Text, builtins.int]:
        """Maps each data point name passed in RegisterDatapointsRequest to a data point id"""
        pass
    def __init__(self,
        *,
        results: typing.Optional[typing.Mapping[typing.Text, builtins.int]] = ...,
        ) -> None: ...
    def ClearField(self, field_name: typing_extensions.Literal["results",b"results"]) -> None: ...
global___RegisterDatapointsReply = RegisterDatapointsReply

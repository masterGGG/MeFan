# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: imgCheck.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='imgCheck.proto',
  package='mifan',
  syntax='proto3',
  serialized_options=_b('H\003'),
  serialized_pb=_b('\n\x0eimgCheck.proto\x12\x05mifan\"&\n\x07imgPush\x12\r\n\x05retry\x18\x01 \x01(\x08\x12\x0c\n\x04info\x18\x02 \x01(\t\"\x1b\n\nimgPopRqst\x12\r\n\x05retry\x18\x01 \x01(\x08\"\x1a\n\nimgPopResp\x12\x0c\n\x04info\x18\x01 \x01(\tB\x02H\x03\x62\x06proto3')
)




_IMGPUSH = _descriptor.Descriptor(
  name='imgPush',
  full_name='mifan.imgPush',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='retry', full_name='mifan.imgPush.retry', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='info', full_name='mifan.imgPush.info', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=25,
  serialized_end=63,
)


_IMGPOPRQST = _descriptor.Descriptor(
  name='imgPopRqst',
  full_name='mifan.imgPopRqst',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='retry', full_name='mifan.imgPopRqst.retry', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=65,
  serialized_end=92,
)


_IMGPOPRESP = _descriptor.Descriptor(
  name='imgPopResp',
  full_name='mifan.imgPopResp',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='info', full_name='mifan.imgPopResp.info', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=94,
  serialized_end=120,
)

DESCRIPTOR.message_types_by_name['imgPush'] = _IMGPUSH
DESCRIPTOR.message_types_by_name['imgPopRqst'] = _IMGPOPRQST
DESCRIPTOR.message_types_by_name['imgPopResp'] = _IMGPOPRESP
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

imgPush = _reflection.GeneratedProtocolMessageType('imgPush', (_message.Message,), {
  'DESCRIPTOR' : _IMGPUSH,
  '__module__' : 'imgCheck_pb2'
  # @@protoc_insertion_point(class_scope:mifan.imgPush)
  })
_sym_db.RegisterMessage(imgPush)

imgPopRqst = _reflection.GeneratedProtocolMessageType('imgPopRqst', (_message.Message,), {
  'DESCRIPTOR' : _IMGPOPRQST,
  '__module__' : 'imgCheck_pb2'
  # @@protoc_insertion_point(class_scope:mifan.imgPopRqst)
  })
_sym_db.RegisterMessage(imgPopRqst)

imgPopResp = _reflection.GeneratedProtocolMessageType('imgPopResp', (_message.Message,), {
  'DESCRIPTOR' : _IMGPOPRESP,
  '__module__' : 'imgCheck_pb2'
  # @@protoc_insertion_point(class_scope:mifan.imgPopResp)
  })
_sym_db.RegisterMessage(imgPopResp)


DESCRIPTOR._options = None
# @@protoc_insertion_point(module_scope)

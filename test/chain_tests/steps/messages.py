# -*- coding: utf-8 -*-

from behave import *
from sure import expect
import _kademlia as k

class _Message(object):
    '''
    This class is used to compare expected with actual messages.
    It used endpoints names instead of address to ease feature
    writing.
    '''

    def __init__(self, endpoint_from, endpoint_to, msg_type):
        '''
        Create a message from `endpoint_from` to `endpoint_to`
        as `msg_type`.
        '''
        self.__endpoint_from = endpoint_from
        self.__endpoint_to = endpoint_to
        self.__type = msg_type

    def __repr__(self):
        '''
        Representation of the message when sure detects a mismatch.
        '''
        return '{0}>{1}:{2}'.format(self.__endpoint_from,
                                    self.__endpoint_to,
                                    self.__type )

    def __eq__(self, other):
        '''
        Compare the message with `other` for equality.
        '''
        if self.__endpoint_from != other.__endpoint_from:
            return False

        if self.__endpoint_to != other.__endpoint_to:
            return False

        return self.__type == other.__type

    def __ne__(self, other):
        '''
        Compare the message with `other` for inequality.
        '''
        return not self.__eq__(other)

def _get_session_name(context, endpoint):
    '''
    Retrieve the session name from its `endpoint` thanks to `context`.
    '''
    return context.sessions[repr(endpoint)][0]

def _get_actual_messages(context):
    '''
    Pop all messages exchanged by the kademlia library
    and translate endpoints address into name
    thanks to `context`.
    '''
    messages = []

    while k.count_messages() != 0:
        m = k.pop_message()
        m = _Message(_get_session_name(context, m.from_endpoint()),
                     _get_session_name(context, m.to_endpoint()),
                     str(m.type()))
        messages.append(m)

    return messages

@then('no message has been sent')
def step_impl(context):
    expect(k.count_messages()).to.equal(0)

@then('following messages have been sent')
def step_impl(context):
    expected = []

    for row in context.table:
        expected.append(_Message(row['from'], row['to'], row['type']))

    expect(_get_actual_messages(context)).to.equal(expected)

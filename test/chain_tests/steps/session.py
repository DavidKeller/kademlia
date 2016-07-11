# -*- coding: utf-8 -*-

from behave import *
import _kademlia as k


def _register_session(context, name, s):
    '''
    Save the session `s` in the dict using its endpoint `name`
    and IPv4 endpoint address (i.e. s.ipv4()).
    '''
    context.sessions[name] = name, s
    context.sessions[repr(s.ipv4())] = name, s


@step('we create a first session "{name}"')
@step('a first session "{name}" has been created')
def step_impl(context, name):
    s = k.FirstSession(context.service,
                       context.listen_ipv4,
                       context.listen_ipv6,
                       k.Id())

    _register_session(context, name, s)


@step('we create a session "{name}" knowing "{peer}"')
@step('a session "{name}" knowing "{peer}" has been created')
def step_impl(context, name, peer):
    s = k.Session(context.service,
                  context.sessions[peer][1].ipv4(),
                  context.listen_ipv4,
                  context.listen_ipv6,
                  k.Id())

    _register_session(context, name, s)

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


@step('we create a first session "{name}" with uid "{uid}"')
@step('a first session "{name}" with uid "{uid}" has been created')
def step_impl(context, name, uid):
    s = k.FirstSession(context.service,
                       context.listen_ipv4,
                       context.listen_ipv6,
                       k.Id(uid))

    _register_session(context, name, s)


@step('we create a session "{name}" with uid "{uid}" knowing "{peer}"')
@step('a session "{name}" with uid "{uid}" knowing "{peer}" has been created')
def step_impl(context, name, peer, uid):
    s = k.Session(context.service,
                  context.sessions[peer][1].ipv4(),
                  context.listen_ipv4,
                  context.listen_ipv6,
                  k.Id(uid))

    _register_session(context, name, s)

# -*- coding: utf-8 -*-

from behave import *
import _kademlia as k

@given('a service has been created')
def step_impl( context ):
    context.service = k.Service()
    context.service_work = k.ServiceWork(context.service)

@given('listen endpoints have been created')
def step_impl(context):
    context.listen_ipv4 = k.Endpoint("0.0.0.0", k.DEFAULT_PORT)
    context.listen_ipv6 = k.Endpoint("::", k.DEFAULT_PORT)

def _register_session(context, name, s):
    '''
    Save the session `s` in the dict using its endpoint `name`
    and IPv4 endpoint address (i.e. s.ipv4()).
    '''
    context.sessions[name] = name, s
    context.sessions[repr(s.ipv4())] = name, s

@when('we create a first session "{name}"')
@given('a first session "{name}" has been created')
def step_impl( context, name ):
    s = k.FirstSession(context.service,
                       context.listen_ipv4,
                       context.listen_ipv6,
                       k.Id())

    _register_session(context, name, s)

@when('we create a session "{name}" knowing "{peer}"')
@given('a session "{name}" knowing "{peer}" has been created')
def step_impl(context, name, peer):
    s = k.Session(context.service,
                  context.sessions[peer][1].ipv4(),
                  context.listen_ipv4,
                  context.listen_ipv6,
                  k.Id())

    _register_session(context, name, s)

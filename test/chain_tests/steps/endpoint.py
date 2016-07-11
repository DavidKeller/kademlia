# -*- coding: utf-8 -*-

from behave import *
import _kademlia as k


@step('listen endpoints have been created')
def step_impl(context):
    context.listen_ipv4 = k.Endpoint("0.0.0.0", k.DEFAULT_PORT)
    context.listen_ipv6 = k.Endpoint("::", k.DEFAULT_PORT)

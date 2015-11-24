# -*- coding: utf-8 -*-

from behave import *
from _kademlia import *

def before_all( context ):
    context.listen_ipv4 = Endpoint( "0.0.0.0", DEFAULT_PORT )
    context.listen_ipv6 = Endpoint( "::", DEFAULT_PORT )
    context.service = Service()

def after_scenario( context, scenario ):
    clear_messages()

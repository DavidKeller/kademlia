# -*- coding: utf-8 -*-

from _kademlia import *

#enable_log_for( '*' )

def before_scenario( context, scenario ):
    context.sessions = dict()

def after_step( context, step ):
    if context.service:
        context.service.poll()

def after_scenario( context, scenario ):
    del context.sessions
    forget_attributed_ip()
    clear_messages()

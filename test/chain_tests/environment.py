# -*- coding: utf-8 -*-

import _kademlia as k

# enable_log_for( '*' )


def before_scenario(context, scenario):
    context.sessions = dict()


def after_step(context, step):
    if context.service:
        context.service.poll()


def after_scenario(context, scenario):
    del context.sessions
    k.forget_attributed_ip()
    k.clear_messages()

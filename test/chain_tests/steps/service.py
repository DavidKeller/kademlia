# -*- coding: utf-8 -*-

from behave import *
import _kademlia as k


@step('a service has been created')
def step_impl(context):
    context.service = k.Service()
    context.service_work = k.ServiceWork(context.service)

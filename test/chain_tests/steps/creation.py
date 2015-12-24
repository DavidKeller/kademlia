# -*- coding: utf-8 -*-

from behave import *
from _kademlia import *

@given( 'a service has been created' )
def step_impl( context ):
    context.service = Service()
    context.service_work = ServiceWork( context.service )

@given( 'listen endpoints have been created' )
def step_impl( context ):
    context.listen_ipv4 = Endpoint( "0.0.0.0", DEFAULT_PORT )
    context.listen_ipv6 = Endpoint( "::", DEFAULT_PORT )

@when( 'we create a first session "{s}"' )
@given( 'a first session "{s}" has been created' )
def step_impl( context, s ):
    context.sessions[ s ] = FirstSession( context.service
                                        , context.listen_ipv4
                                        , context.listen_ipv6
                                        , Id() )

@when( 'we create a session "{s}" knowing "{b}"' )
@given( 'a session "{s}" knowing "{b}" has been created' )
def step_impl( context, s, b ):
    context.sessions[ s ] = Session( context.service
                                   , context.sessions[ b ].ipv4()
                                   , context.listen_ipv4
                                   , context.listen_ipv6
                                   , Id() )


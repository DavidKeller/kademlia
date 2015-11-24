# -*- coding: utf-8 -*-

from behave import *
from _kademlia import *

@given( 'no session has been created' )
def step_impl( context ):
    pass

@when( 'we create a first session' )
def step_impl( context ):
    context.session = FirstSession( context.service
                                  , context.listen_ipv4
                                  , context.listen_ipv6
                                  , Id() )

@then( 'no message has been sent' )
def step_impl( context ):
    assert count_messages() is 0

#first_session = FirstSession( service
#                            , listen_ipv4
#                            , listen_ipv6
#                            , Id())
#
#session1 = Session( service
#                   , first_session.ipv4()
#                   , listen_ipv4, listen_ipv6
#                   , Id())
#
#session2 = Session( service
#                  , first_session.ipv4()
#                  ,  listen_ipv4, listen_ipv6
#                  ,  Id() )
#
#def on_save(e):
#    if e:
#        print 'Can\'t save ({0})'.format(e)
#    else:
#        print 'Saved'
#
#session1.async_save( "m1", "content", on_save )
#
#def on_load(e, d):
#    if e:
#        print 'Can\'t load ({0})'.format(e)
#    else:
#        print 'Loaded: \'{0}\''.format(d)
#
#session2.async_load( "m0", on_load )
#session2.async_load( "m1", on_load )
#
#while count_messages() > 0:
#    m = pop_message()
#    print m
#
#    if m == Message( Endpoint( "10.0.0.2", DEFAULT_PORT )
#                   , Endpoint( "10.0.0.1", DEFAULT_PORT )
#                   , MessageType.PING_REQUEST ):
#        print "lol"

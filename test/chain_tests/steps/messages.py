# -*- coding: utf-8 -*-

from behave import *
from _kademlia import *

@then( 'no message has been sent' )
def step_impl( context ):
    count = count_messages()
    assert count == 0, '0 != {0}'.format( count )

def to_message_type( t ):
    return eval( 'MessageType.{0}'.format( t ) )

@then( 'following messages have been sent' )
def step_impl( context ):
    for row in context.table:
        expected = Message( Endpoint( row['from'].encode(), DEFAULT_PORT )
                          , Endpoint( row['to'].encode(), DEFAULT_PORT )
                          , to_message_type( row['type'].encode() ) )
        actual = pop_message()

        assert expected == actual,'"{0}" != "{1}"'.format( expected, actual )

    assert count_messages() == 0, 'unexpected "{0}" message left'.format( pop_message() )

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
#

Feature: boostrap sessions
  In order to discover more surrounding peers, session
  will query for neighbors when started

  Background: API has been initialized
    Given a service has been created
    And listen endpoints have been created

  Scenario: start the first session
    When we create a first session "a"
    Then no message has been sent

  Scenario: start the second session
    Given a first session "a" has been created
    When we create a session "b" knowing "a"
    Then following messages have been sent
      | from | to | type                |
      | b    | a  | FIND_PEER_REQUEST   |
      | a    | b  | FIND_PEER_RESPONSE  |

  Scenario: start the third session
    Given a first session "a" has been created
    And a session "b" knowing "a" has been created
    When we create a session "c" knowing "a"
    Then following messages have been sent
      | from | to | type                |
      | b    | a  | FIND_PEER_REQUEST   |
      | a    | b  | FIND_PEER_RESPONSE  |
      | c    | a  | FIND_PEER_REQUEST   |
      | a    | c  | FIND_PEER_RESPONSE  |

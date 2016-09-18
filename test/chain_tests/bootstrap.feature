Feature: boostrap sessions
  In order to discover more surrounding peers, session
  will query for neighbors when started

  Background: API has been initialized
    Given a service has been created
    And listen endpoints have been created

  Scenario: A unique session is created
    When we create a first session "a" with uid "0"
    Then no message has been sent

  Scenario: Two sessions are created
    Given a first session "a" with uid "8000000000000000000000000000000000000000" has been created
    When we create a session "b" with uid "4000000000000000000000000000000000000000" knowing "a"
    Then following messages have been sent
      | from | to | type                |
      | b    | a  | FIND_PEER_REQUEST   | # Initial lookup
      | a    | b  | FIND_PEER_RESPONSE  |
      | b    | b  | FIND_PEER_REQUEST   | # Bucket refresh
      | b    | a  | FIND_PEER_REQUEST   | # Bucket refresh
      | b    | b  | FIND_PEER_RESPONSE  |
      | a    | b  | FIND_PEER_RESPONSE  |

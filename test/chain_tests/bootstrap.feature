Feature: Boostrap sessions

Scenario: Start the first session
    Given no session has been created
    When we create a first session
    Then no message has been sent

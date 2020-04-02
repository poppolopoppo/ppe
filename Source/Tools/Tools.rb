# frozen_string_literal: true

$Build.namespace(:Tools) do
    include!(
        'BuildRobot/BuildRobot.rb',
        'UnitTest/UnitTest.rb' )
end

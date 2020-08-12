# frozen_string_literal: true

module ANSI

    Codes = {
        reset: "\u001b[0m",

        bold: "\u001b[1m",
        faint:  "\u001b[2m",
        italic: "\u001b[3m",
        underline:  "\u001b[4m",
        blink0: "\u001b[5m",
        blink1: "\u001b[6m",
        reversed: "\u001b[7m",

        frame: "\u001b[51m",
        encircle: "\u001b[52m",
        overline: "\u001b[53m",

        fg0_black: "\u001b[30m",
        fg0_red: "\u001b[31m",
        fg0_green: "\u001b[32m",
        fg0_yellow: "\u001b[33m",
        fg0_blue: "\u001b[34m",
        fg0_magenta: "\u001b[35m",
        fg0_cyan: "\u001b[36m",
        fg0_white: "\u001b[37m",

        fg1_black: "\u001b[30;1m",
        fg1_red: "\u001b[31;1m",
        fg1_green: "\u001b[32;1m",
        fg1_yellow: "\u001b[33;1m",
        fg1_blue: "\u001b[34;1m",
        fg1_magenta: "\u001b[35;1m",
        fg1_cyan: "\u001b[36;1m",
        fg1_white: "\u001b[37;1m",

        bg0_black: "\u001b[40m",
        bg0_red: "\u001b[41m",
        bg0_green: "\u001b[42m",
        bg0_yellow: "\u001b[43m",
        bg0_blue: "\u001b[44m",
        bg0_magenta: "\u001b[45m",
        bg0_cyan: "\u001b[46m",
        bg0_white: "\u001b[47m",

        bg1_black: "\u001b[40;1m",
        bg1_red: "\u001b[41;1m",
        bg1_green: "\u001b[42;1m",
        bg1_yellow: "\u001b[43;1m",
        bg1_blue: "\u001b[44;1m",
        bg1_magenta: "\u001b[45;1m",
        bg1_cyan: "\u001b[46;1m",
        bg1_white: "\u001b[47;1m",

        kill_line: "\u001b[K",
        cursor_up: "\u001b[A",
    }

    All_colors = [ :black, :red, :green, :yellow, :blue, :magenta, :cyan ]

    def self.random_color(prefix)
        return "#{prefix}_#{All_colors[rand(All_colors.length)]}".to_sym
    end

    def self.colors(enabled)
        if enabled
            return ANSI::Codes
        else
            fakeCodes = {}
            ANSI::Codes.keys.each{|x| fakeCodes[x] = ''}
            return fakeCodes
        end
    end

end #~ ANSI

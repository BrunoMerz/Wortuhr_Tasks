#pragma once


class DisplayModes {
    public:
        static DisplayModes* getInstance();

        void displayAnsage(void);
        void displayWeekday(void);
        void displayDate(void);
        void displayMoonphase(void);
        void displayWeather(void);
        void displayExtTemp(void);
        void displayExtHumidity(void);
        void displaySeconds(void);

    private:
        DisplayModes(void);
        static DisplayModes *instance;
};
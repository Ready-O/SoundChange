#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //Main window constructor
    MainWindow(QWidget *parent = nullptr);

    //Main window destructor
    ~MainWindow();

private slots:

    /** Change the state of the buttons (Enabled/ Not enabled)
     * @param state
     */
    void change_state_buttons(bool state);

    /** Change the state of the effect buttons and sliders (Enabled/ Not enabled)
     * @param state
     */
    void change_state_effects(bool state);

    /**
     * Slot performed when the play button is clicked
     * It pauses when the player is playing and it plays if the player
     * is in pause state.
     * It can also play the selected audio when the player is stopped.
     */
    void on_PlayButton_clicked();

    /**
     * Change the state of the playing button depending on the state
     * of the player
     */
    void state_play(bool playing);

    /**
     * Slot performed when the stop button is clicked
     */
    void on_StopButton_clicked();

    /**
     * Slot performed when an item in the playlist is clicked
     * When a file is clicked, you can choose many options
     */
    void on_playlist_itemClicked(QListWidgetItem *item);

    /**
     * Slot performed when an item in the playlist is double-clicked
     * When an item is doubleclicked, the player will play this item
     * and if the item was a Wav file, you could apply effects on it
     */
    void doubleClickAction(QListWidgetItem *item);

    /**
     * Slot performed when the next button is clicked
     * It plays the next item in the playlist
     */
    void on_NextButton_clicked();

    /**
     * Slot performed when the back button is clicked
     * It plays the previous item in the playlist
     */
    void on_BackButton_clicked();


    /**
     * Slot performed when the mute button is clicked
     * If the player is muted, it unmutes it and vice-versa
     */
    void on_MuteButton_clicked();

    /**
     * Slot performed when the volume slider was moved
     */
    void on_Volume_valueChanged(int value);

    /**
     * Slot performed when the open action is triggered
     * Add to the playlist the specified audio files in input
     * Possible formats : .wav/.mp3/.ogg
     */
    void on_actionOpen_triggered();

    /**
     * Add to playlist a list of strings.
     * A string here is a full path of an audio file
     * The item added to the playlist will have the name of the audio as text
     * and the file itself as data
     */
    void add_to_playlist(QStringList input_files);

    /**
     * Returns the data (QFile* in this case) of an item in the playlist
     */
    QFile* extractData(QListWidgetItem *item);

    /**
     * Add the audio of the current selected item in the playlist
     * to the player as a media
     */
    void add_media();

    /**
     * Delete the selected item in the playlist
     * If the item selected is actually played by the player, the player stops
     */
    void on_Delete_clicked();

    /** When the player sends the signal QMediaPlayer::durationChanged,
     * it takes the duration given by the signal and print it in the UI
     * and set it as the maximum range of the player slider.
     *
     * The duration given by the signal is in milliseconds
     */
    void durationChanged(qint64 duration);

    /** When the player sends the signal QMediaPlayer::positionChanged,
     * it takes the duration given by the signal and print it in the UI
     * and set it as the current position of the player slider.
     *
     * The duration given by the signal is in milliseconds
     */
    void positionChanged(qint64 duration);

    /**
     * Slot performed when the playing slider is moved
     */
    void on_SliderAudio_sliderMoved(int position);

    /**
     * Slot performed when the repeat checkbox is checked
     */
    void on_RepeatOption_clicked();

    /**
     * If the player is in state EndOfMedia, replay the audio
     */
    void checkRepeat(QMediaPlayer::MediaStatus status);

    /**
     * Slot performed when the export button is clicked
     */
    void on_ExportButton_clicked();

    /**
     * Slot performed when the tempo slider is moved
     */
    void on_SliderTempo_valueChanged(int value);

    /**
     * Slot performed when the tempo slider is moved and released
     * Change tempo of the audio
     */
    void on_SliderTempo_sliderReleased();

    /**
     * Slot performed when the pitch slider is moved
     */
    void on_SliderPitch_valueChanged(int value);

    /**
     * Slot performed when the pitch slider is moved and released
     * Change pitch of the audio
     */
    void on_SliderPitch_sliderReleased();

    /**
     * Genrates a new audio file with tempo and pitch in input using
     * the soundstretch command
     */
    void generate_audio_with_effect(QString input,int tempoValue,int pitchValue);

    /**
     * When applying effects (by changing tempo and pitch),
     * the player plays the generated temporary file and plays it
     * in the exact position the original file was.
     *
     * Since the tempo changes the duration of the audio, we save the progress
     * of the original file as percentages (rather than seconds)
     */
    void switch_to_temp(int tempo,int pitch);

    /**
     * Slot performed when about action is triggered
     */
    void on_actionAbout_triggered();

    /**
     * Slot performed when quit action is triggered
     */
    void on_actionQuit_triggered();

private:
    // The Main Window
    Ui::MainWindow *ui;

    //The player used to play the audio files
    QMediaPlayer *player;

    //Bool defining if the player is playing or not
    bool playing = false;

    //Bool defining if the volume is muted or not
    bool is_muted = false;

    //Bool defining if player plays an audio repeatedly or not
    bool repeat = false;

    //Name of the current File set in the player
    QString current_media_in_player;

    //Current item chosen in the playlist
    QListWidgetItem *current_item;

    //Bool defining if a temporary file was generated or not
    bool temp_generated =false;

    /** Saving the last tempo,
     * used when the player wants to play a file after applying an effect
     */
    int previous_tempo = 100;
};
#endif // MAINWINDOW_H

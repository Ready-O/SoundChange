#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMediaPlayer>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QProcess>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //To disable the "Maximize Window" option
    this->setFixedSize(this->width(),this->height());


    //We create the player (of class QMediaPlayer) which is the main widget used
    player = new QMediaPlayer(this);

    //The buttons and effects are initialized (not clickable without audio selected)
    change_state_buttons(false);
    change_state_effects(false);

    //The Volume is initialized
    ui->Volume->setRange(0, 100);
    ui->Volume->setValue(100);

    //The main Slider is initialized
    ui->SliderAudio->setRange(0, 0);
    ui->duration_played->setText("00:00");
    ui->total_duration->setText("/ 00:00");

    //The effect sliders are initialized
    ui->SliderTempo->setRange(-50, 200);
    ui->TempoValue->setText("100 %");
    ui->SliderPitch->setRange(-10,10);
    ui->PitchValue->setText("0 semitones");

    //Signals sent by the player to the MainWindow in order to change the main slider
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::durationChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::positionChanged);

    //Signals sent by QMediaPlayer to the MainWindow when it stops playing
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::checkRepeat);

    //When we double-click an audio in the playlist, play it
    connect(ui->playlist, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doubleClickAction(QListWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete ui;

    // We remove the temporary file when destroying the main window
    QFile::remove("temp.wav");
}

/** Change the state of the buttons (Enabled/ Not enabled)
 * @param state
 */
void MainWindow::change_state_buttons(bool state){
    ui->PlayButton->setEnabled(state);
    ui->StopButton->setEnabled(state);
    ui->NextButton->setEnabled(state);
    ui->BackButton->setEnabled(state);
    ui->Delete->setEnabled(state);
}

/** Change the state of the effect buttons and sliders (Enabled/ Not enabled)
 * @param state
 */
void MainWindow::change_state_effects(bool state){
    ui->ExportButton->setEnabled(state);
    ui->SliderTempo->setEnabled(state);
    ui->SliderPitch->setEnabled(state);
    if (!state){
        ui->SliderTempo->setValue(0);
        ui->SliderPitch->setValue(0);
    }
}

/**
 * Slot performed when the play button is clicked
 * It pauses when the player is playing and it plays if the player
 * is in pause state.
 * It can also play the selected audio when the player is stopped.
 */
void MainWindow::on_PlayButton_clicked()
{
    ui->title_playing->setText("Playing  :  "+current_item ->text());
    if (playing){
        player->pause();
        playing = false;
    }
    else{
       player->play();
       playing = true;
    }
    state_play(playing);

    // The following code execute when the player has no audio or is stopped,
    //the slot plays the selected audio in the playlist
    if (current_item != nullptr){
        if((player->mediaStatus()==QMediaPlayer::NoMedia )||(player->mediaStatus()==QMediaPlayer::LoadedMedia)){
            doubleClickAction(current_item);
        }
    }
}

/**
 * Change the state of the playing button depending on the state
 * of the player
 */
void MainWindow::state_play(bool playing){
    if (!playing){
        ui->PlayButton->setIcon(QIcon(":/icons/icons/play-32.png"));
        ui->PlayButton->setText("Play");
    }
    else{
        ui->PlayButton->setIcon(QIcon(":/icons/icons/pause-32.png"));
        ui->PlayButton->setText("Pause");
    }
}

/**
 * Slot performed when the stop button is clicked
 */
void MainWindow::on_StopButton_clicked()
{
    player->stop();
    ui->title_playing->setText("");
    ui->cannot_label->setText("");
    playing = false;
    state_play(playing);
    change_state_effects(false);
    change_state_buttons(false);
    current_item = nullptr;
}

/**
 * Slot performed when an item in the playlist is clicked
 * When an item is clicked, you can choose many options
 */
void MainWindow::on_playlist_itemClicked(QListWidgetItem *item)
{
    current_item = item;
    change_state_buttons(true);
}

/**
 * Slot performed when an item in the playlist is double-clicked
 * When an item is doubleclicked, the player will play this item
 * and if the item was a Wav file, you could apply effects on it
 */
void MainWindow::doubleClickAction(QListWidgetItem *item)
{
    on_playlist_itemClicked(item);
    add_media();
    QRegExp rx("[.]");// match a dot
    QStringList list = item->text().split(rx);
    QString codec = list.at(list.size()-1);
    if (codec == "wav"){
        change_state_effects(true);
        ui->cannot_label->setText("");
        ui->SliderTempo->setValue(0);
        ui->SliderPitch->setValue(0);
    }
    else{
        change_state_effects(false);
        ui->cannot_label->setText("You cannot apply effects on non-Wav files.");
    }
    temp_generated = false;
    on_PlayButton_clicked();
}

/**
 * Slot performed when the next button is clicked
 * It plays the next item in the playlist
 */
void MainWindow::on_NextButton_clicked()
{
    if (current_item != nullptr){
        //Sachant que la première row est de valeur 0
        int row = ui->playlist->row(current_item) + 1;
        int max_row = ui->playlist->count();
        if (row == max_row){
            row = 0;
        }
        ui->playlist->setCurrentRow(row);
        current_item = ui->playlist->currentItem();
        doubleClickAction(current_item);
    }
}

/**
 * Slot performed when the back button is clicked
 * It plays the previous item in the playlist
 */
void MainWindow::on_BackButton_clicked()
{
    if (current_item != nullptr){
        int row = ui->playlist->row(current_item) - 1;
        if (row == -1){
            row = ui->playlist->count()-1;
        }
        ui->playlist->setCurrentRow(row);
        current_item = ui->playlist->currentItem();
        doubleClickAction(current_item);
    }
}

/**
 * Slot performed when the mute button is clicked
 * If the player is muted, it unmutes it and vice-versa
 */
void MainWindow::on_MuteButton_clicked()
{
    if (!is_muted){
        player->setMuted(true);
        is_muted = true;
        ui->MuteButton->setIcon(QIcon(":/icons/icons/mute-32.png"));
    }
    else{
        player->setMuted(false);
        is_muted = false;
        ui->MuteButton->setIcon(QIcon(":/icons/icons/volume_up-32.png"));
    }
}

/**
 * Slot performed when the volume slider is moved
 */
void MainWindow::on_Volume_valueChanged(int value)
{
    player->setVolume(value);
}

/**
 * Slot performed when the open action is triggered
 * Add to the playlist the specified audio files in input
 * Possible formats : .wav/.mp3/.ogg
 */
void MainWindow::on_actionOpen_triggered()
{
    QString filter = tr("All Supported Files (*.wav *.mp3 *.ogg);;"
                "Waveform Audio File Format Files : *.wav (*.wav);;"
                        "MPEG-1 Audio Layer 3 Files : *.mp3 (*.mp3);;"
                        "Ogg Files : *.ogg (*.ogg);;"
                        );
    QStringList input_files = QFileDialog::getOpenFileNames(this, "Open an audio file",QString(), filter);
    add_to_playlist(input_files);
}

/**
 * Add to playlist a list of strings.
 * A string here is a full path of an audio file
 * The item added to the playlist will have the name of the audio as text
 * and the file itself as data
 */
void MainWindow::add_to_playlist(QStringList input_files){
    foreach(QString input, input_files){
        QListWidgetItem *newItem = new QListWidgetItem;
        QVariant fullPath(input);

        QFileInfo info(input);
        newItem->setText(info.baseName()+"."+info.suffix());
        newItem->setData(Qt::UserRole, fullPath);
        ui->playlist->addItem(newItem);
    }
}

/**
 * Returns the data (QFile* in this case) of an item in the playlist
 */
QFile* MainWindow::extractData(QListWidgetItem *item){
    QString fullPath = item ->data(Qt::UserRole).toString();
    QFile *audio = new QFile(fullPath);
    return audio;
}

/**
 * Add the audio of the current selected item in the playlist
 * to the player as a media
 */
void MainWindow::add_media(){
    if (current_item != nullptr){
        QFile *audio = extractData(current_item);
        audio->open(QIODevice::ReadOnly);
        player->setMedia(0, audio);
        current_media_in_player = current_item->text();
        playing = false;
    }
}


/**
 * Delete the selected item in the playlist
 * If the item selected is actually played by the player, the player stops
 */
void MainWindow::on_Delete_clicked()
{
    if (current_item != nullptr){
       ui->playlist->takeItem(ui->playlist->row(current_item));
       if (current_media_in_player == current_item->text()){
            on_StopButton_clicked();
       }
    }
}


/** When the player sends the signal QMediaPlayer::durationChanged,
 * it takes the duration given by the signal and print it in the UI
 * and set it as the maximum range of the player slider.
 *
 * The duration given by the signal is in milliseconds
 */
void MainWindow::durationChanged(qint64 duration)
{
    ui->SliderAudio->setMaximum(duration/1000);
    QTime time(0,(duration / (60 * 1000)) % 60,(duration/1000) % 60);
    QString format = "mm:ss";
    ui->total_duration->setText("/ "+time.toString(format));
}

/** When the player sends the signal QMediaPlayer::positionChanged,
 * it takes the duration given by the signal and print it in the UI
 * and set it as the current position of the player slider.
 *
 * The duration given by the signal is in milliseconds
 */
void MainWindow::positionChanged(qint64 duration)
{
    ui->SliderAudio->setValue(duration/1000);
    QTime time(0,(duration / (60 * 1000)) % 60,(duration/1000) % 60);
    QString format = "mm:ss";
    ui->duration_played->setText(time.toString(format));
}

/**
 * Slot performed when the playing slider is moved
 */
void MainWindow::on_SliderAudio_sliderMoved(int position)
{
    player->setPosition(position*1000);
}

/**
 * Slot performed when the repeat checkbox is checked
 */
void MainWindow::on_RepeatOption_clicked()
{
    if (repeat){
        repeat = false;
    }
    else{
        repeat =true;
    }
}

/**
 * If the player is in state EndOfMedia, replay the audio
 */
void MainWindow::checkRepeat(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia){
        playing = false;
        if (repeat){
            on_PlayButton_clicked();
        }
        else{
            on_NextButton_clicked();
        }
    }

}

/**
 * Slot performed when the export button is clicked
 */
void MainWindow::on_ExportButton_clicked()
{
    QString filter ="Waveform Audio File Format Files (*.wav);;";
    QString name = QFileDialog::getSaveFileName(this, "Save file as", QString(), filter);
    if (temp_generated){
        QFile::copy("temp.wav", name);
    }
    else{
        QFile::copy(extractData(current_item)->fileName(), name);
    }
}

/**
 * Slot performed when the tempo slider is moved
 */
void MainWindow::on_SliderTempo_valueChanged(int value)
{
    ui->TempoValue->setText(QString::number(value+100)+" %");
}

/**
 * Slot performed when the tempo slider is moved and released
 * Change tempo of the audio
 */
void MainWindow::on_SliderTempo_sliderReleased()
{
    int tempo = ui->SliderTempo->value();
    int pitch = ui->SliderPitch->value();
    switch_to_temp(tempo,pitch);
}

/**
 * Slot performed when the pitch slider is moved
 */
void MainWindow::on_SliderPitch_valueChanged(int value)
{
    ui->PitchValue->setText(QString::number(value)+" semitones");
}

/**
 * Slot performed when the pitch slider is moved and released
 * Change pitch of the audio
 */
void MainWindow::on_SliderPitch_sliderReleased()
{
    int tempo = ui->SliderTempo->value();
    int pitch = ui->SliderPitch->value();
    switch_to_temp(tempo,pitch);
}

/**
 * Genrates a new audio file with tempo and pitch in input using
 * the soundstretch command
 */
void MainWindow::generate_audio_with_effect(QString input,int tempoValue,int pitchValue){
    QProcess process;
    QString output = "temp.wav";
    QString tempoArg = "-tempo=" + QString::number(tempoValue);
    QString pitchArg = "-pitch=" + QString::number(pitchValue);
    QStringList arguments;
    arguments << input << output << tempoArg << pitchArg;
    process.start("soundstretch",arguments);
    process.waitForFinished();
    process.close();
}

/**
 * When applying effects (by changing tempo and pitch),
 * the player plays the generated temporary file and plays it
 * in the exact position the original file was.
 *
 * Since the tempo changes the duration of the audio, we save the progress
 * of the original file as percentages (rather than seconds)
 */
void MainWindow::switch_to_temp(int tempo,int pitch){

    QString input = current_item ->data(Qt::UserRole).toString();
    generate_audio_with_effect(input,tempo,pitch);
    temp_generated = true;
    int maximum = ui->SliderAudio->maximum();
    float progress_percentage = (float)ui->SliderAudio->value()/(float)maximum;
        QFile* temp1= new QFile("temp.wav");
        temp1->open(QIODevice::ReadOnly);
        player->setMedia(0, temp1);
    if (playing){
        playing = false;
    }
    on_PlayButton_clicked();
    //float new_total_duration = (float)maximum - (float)maximum * (float)tempo /(float)(tempo + previous_tempo);
    //float new_total_duration = (float)maximum - (float)maximum * ((float)tempo+100-previous_tempo) /(float)previous_tempo;
    float new_total_duration = (float)maximum * ((float)previous_tempo/(float)(tempo +100));
    float new_progress = new_total_duration*(float)progress_percentage;
    on_SliderAudio_sliderMoved((int)new_progress + 1);
    previous_tempo = ui->SliderTempo->value() + 100;
}


/**
 * Slot performed when about action is triggered
 */
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(
        this,
        tr("About"),
        tr("SoundChange<br>by Reda Aoutem<br><br>An audio player that can change <br>"
           "the tempo and pitch of WAV audio."
           "<br><br>Icons by <a "
           "href=\"http://www.visualpharm.com/\">visualpharm</a>"
            "<br><br>Library used : <a "
           "href=\"https://www.surina.net/soundtouch/\">SoundTouch</a>"));

}

/**
 * Slot performed when quit action is triggered
 */
void MainWindow::on_actionQuit_triggered()
{
    close();
}

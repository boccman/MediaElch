#include "MovieWidget.h"
#include "ui_MovieWidget.h"

#include <QDoubleValidator>
#include <QIntValidator>
#include <QMovie>
#include <QPainter>
#include <QScrollBar>
#include "Manager.h"
#include "MovieImageDialog.h"
#include "MovieSearch.h"
#include "QuestionDialog.h"

MovieWidget::MovieWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MovieWidget)
{
    ui->setupUi(this);
    ui->rating->setValidator(new QDoubleValidator);
    ui->runtime->setValidator(new QIntValidator);
    ui->playcount->setValidator(new QIntValidator);
    ui->movieName->clear();
    ui->actors->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->genres->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->countries->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->studios->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    QFont font = ui->movieName->font();
    font.setPointSize(font.pointSize()+4);
    ui->movieName->setFont(font);

    font = ui->posterResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->posterResolution->setFont(font);
    ui->backdropResolution->setFont(font);

    m_progressMessageId = 10001;
    m_movie = 0;
    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->poster, SIGNAL(clicked()), this, SLOT(chooseMoviePoster()));
    connect(ui->backdrop, SIGNAL(clicked()), this, SLOT(chooseMovieBackdrop()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(posterDownloadFinished(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadProgress(DownloadManagerElement)), this, SLOT(posterDownloadProgress(DownloadManagerElement)));
    connect(m_posterDownloadManager, SIGNAL(downloadsLeft(int)), this, SLOT(actorDownloadsLeft(int)));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(movieNameChanged(QString)));
    connect(ui->buttonAddActor, SIGNAL(clicked()), this, SLOT(addActor()));
    connect(ui->buttonRemoveActor, SIGNAL(clicked()), this, SLOT(removeActor()));
    connect(ui->buttonAddCountry, SIGNAL(clicked()), this, SLOT(addCountry()));
    connect(ui->buttonRemoveCountry, SIGNAL(clicked()), this, SLOT(removeCountry()));
    connect(ui->buttonAddGenre, SIGNAL(clicked()), this, SLOT(addGenre()));
    connect(ui->buttonRemoveGenre, SIGNAL(clicked()), this, SLOT(removeGenre()));
    connect(ui->buttonAddStudio, SIGNAL(clicked()), this, SLOT(addStudio()));
    connect(ui->buttonRemoveStudio, SIGNAL(clicked()), this, SLOT(removeStudio()));
    connect(ui->groupBox_3, SIGNAL(resized(QSize)), this, SLOT(groupBoxResized(QSize)));

    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->originalName, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->set, SIGNAL(currentIndexChanged(int)), this, SLOT(markHasChanged()));
    connect(ui->set, SIGNAL(editTextChanged(QString)), this, SLOT(markHasChanged()));
    connect(ui->tagline, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->rating, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->released, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(markHasChanged()));
    connect(ui->runtime, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(markHasChanged()));
    connect(ui->certification, SIGNAL(currentIndexChanged(int)), this, SLOT(markHasChanged()));
    connect(ui->trailer, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->playcount, SIGNAL(textEdited(QString)), this, SLOT(markHasChanged()));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(markHasChanged()));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(markHasChanged()));
    connect(ui->actors, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(markHasChanged()));
    connect(ui->genres, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(markHasChanged()));
    connect(ui->countries, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(markHasChanged()));
    connect(ui->studios, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(markHasChanged()));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();

    this->setDisabledTrue();
    this->clear();

    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    QString language = QLocale::system().name().left(2);
    QString textStartMovieWidget = ":/img/text_start_movieWidget_" + language + ".png";
    QFileInfo fi(textStartMovieWidget);
    if (!fi.exists())
        textStartMovieWidget = ":/img/text_start_movieWidget_en.png";
    QPixmap firstTimePixmap(textStartMovieWidget);
    m_firstTimeLabel = new QLabel(this);
    m_firstTimeLabel->setStyleSheet("border-radius: 5px; background-color: rgba(0, 0, 0, 150); padding: 20px;");
    m_firstTimeLabel->setPixmap(firstTimePixmap);
    m_firstTimeLabel->resize(firstTimePixmap.width()+40, firstTimePixmap.height()+40);
    m_firstTimeLabel->hide();

    m_hasChanged = false;
}

MovieWidget::~MovieWidget()
{
    delete ui;
}

void MovieWidget::groupBoxResized(QSize size)
{
    m_firstTimeLabel->move((size.width()-m_firstTimeLabel->width())/2, (size.height()-m_firstTimeLabel->height())/2);
}

void MovieWidget::showFirstTime()
{
    m_firstTimeLabel->show();
}

void MovieWidget::hideFirstTime()
{
    m_firstTimeLabel->hide();
}

void MovieWidget::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

void MovieWidget::addActor()
{
    int row = ui->actors->rowCount();
    ui->actors->insertRow(row);
    ui->actors->setItem(row, 0, new QTableWidgetItem(tr("Unknown Actor")));
    ui->actors->setItem(row, 1, new QTableWidgetItem(tr("Unkown Role")));
    ui->actors->scrollToBottom();
    m_hasChanged = true;
}

void MovieWidget::removeActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected())
        return;
    ui->actors->removeRow(row);
    m_hasChanged = true;
}

void MovieWidget::addGenre()
{
    int row = ui->genres->rowCount();
    ui->genres->insertRow(row);
    ui->genres->setItem(row, 0, new QTableWidgetItem(tr("Unkown Genre")));
    ui->genres->scrollToBottom();
    m_hasChanged = true;
}

void MovieWidget::removeGenre()
{
    int row = ui->genres->currentRow();
    if (row < 0 || row >= ui->genres->rowCount() || !ui->genres->currentItem()->isSelected())
        return;
    ui->genres->removeRow(row);
    m_hasChanged = true;
}

void MovieWidget::addStudio()
{
    int row = ui->studios->rowCount();
    ui->studios->insertRow(row);
    ui->studios->setItem(row, 0, new QTableWidgetItem(tr("Unknown Studio")));
    ui->studios->scrollToBottom();
    m_hasChanged = true;
}

void MovieWidget::removeStudio()
{
    int row = ui->studios->currentRow();
    if (row < 0 || row >= ui->studios->rowCount() || !ui->studios->currentItem()->isSelected())
        return;
    ui->studios->removeRow(row);
    m_hasChanged = true;
}

void MovieWidget::addCountry()
{
    int row = ui->countries->rowCount();
    ui->countries->insertRow(row);
    ui->countries->setItem(row, 0, new QTableWidgetItem(tr("Unkown Country")));
    ui->countries->scrollToBottom();
    m_hasChanged = true;
}

void MovieWidget::removeCountry()
{
    int row = ui->countries->currentRow();
    if (row < 0 || row >= ui->countries->rowCount() || !ui->countries->currentItem()->isSelected())
        return;
    ui->countries->removeRow(row);
    m_hasChanged = true;
}

void MovieWidget::clear()
{
    ui->set->clear();
    ui->certification->clear();
    ui->movieName->clear();
    ui->files->clear();
    ui->name->clear();
    ui->originalName->clear();
    ui->tagline->clear();
    ui->rating->clear();
    ui->released->setDate(QDate::currentDate());
    ui->runtime->clear();
    ui->trailer->clear();
    ui->playcount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->overview->clear();
    ui->actors->setRowCount(0);
    ui->genres->setRowCount(0);
    ui->studios->setRowCount(0);
    ui->countries->setRowCount(0);
    ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
    ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->tabWidget->setCurrentIndex(0);
    ui->posterResolution->setText("");
    ui->backdropResolution->setText("");
    m_chosenPoster = QImage();
    m_chosenBackdrop = QImage();
    m_loadedFromScraper = false;
}

void MovieWidget::movieNameChanged(QString text)
{
    ui->movieName->setText(text);
}

void MovieWidget::setEnabledTrue()
{
    ui->groupBox_3->setEnabled(true);
    emit setActionSaveEnabled(true);
    emit setActionSearchEnabled(true);
}

void MovieWidget::setDisabledTrue()
{
    ui->groupBox_3->setDisabled(true);
    emit setActionSaveEnabled(false);
    emit setActionSearchEnabled(false);
}

void MovieWidget::setMovie(Movie *movie)
{
    if (m_hasChanged) {
        if (QuestionDialog::instance()->exec() != QDialog::Accepted) {
            emit movieChangeCanceled();
            return;
        }
    }
    movie->loadData(Manager::instance()->mediaCenterInterface());
    hideFirstTime();
    m_posterDownloadManager->abortDownloads();
    m_movie = movie;
    movie->loadImages(Manager::instance()->mediaCenterInterface());
    this->updateMovieInfo();
    m_hasChanged = false;
}

void MovieWidget::startScraperSearch()
{
    if (m_movie == 0)
        return;
    emit setActionSearchEnabled(false);
    emit setActionSaveEnabled(false);
    MovieSearch::instance()->exec(m_movie->name());
    if (MovieSearch::instance()->result() == QDialog::Accepted) {
        this->setDisabledTrue();
        m_movie->loadData(MovieSearch::instance()->scraperId(), Manager::instance()->scrapers().at(MovieSearch::instance()->scraperNo()));
        connect(this->m_movie, SIGNAL(loaded()), this, SLOT(loadDone()), Qt::UniqueConnection);
        m_hasChanged = true;
    } else {
        emit setActionSearchEnabled(true);
        emit setActionSaveEnabled(true);
    }
}

void MovieWidget::loadDone()
{
    if (m_movie == 0)
        return;

    this->updateMovieInfo();
    this->setEnabledTrue();
    m_loadedFromScraper = true;

    if (m_movie->posters().size() > 0) {
        emit setActionSaveEnabled(false);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = m_movie->posters().at(0).originalUrl;
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
    }

    if (m_movie->backdrops().size() > 0) {
        emit setActionSaveEnabled(false);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = m_movie->backdrops().at(0).originalUrl;
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
    }
}

void MovieWidget::updateMovieInfo()
{
    if (m_movie == 0)
        return;

    this->clear();
    ui->set->clear();
    ui->certification->clear();

    ui->files->setText(m_movie->files().join(", "));
    ui->files->setToolTip(m_movie->files().join("\n"));
    ui->name->setText(m_movie->name());
    ui->movieName->setText(m_movie->name());
    ui->originalName->setText(m_movie->originalName());
    ui->tagline->setText(m_movie->tagline());
    ui->rating->setText(QString::number(m_movie->rating()));
    ui->released->setDate(m_movie->released());
    ui->runtime->setText(QString::number(m_movie->runtime()));
    ui->trailer->setText(m_movie->trailer().toString());
    ui->playcount->setText(QString::number(m_movie->playcount()));
    ui->lastPlayed->setDateTime(m_movie->lastPlayed());
    ui->overview->setHtml(m_movie->overview());
    ui->watched->setChecked(m_movie->watched());

    QStringList certifications;
    QStringList sets;
    sets.append("");
    certifications.append("");
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (!sets.contains(movie->set()) && !movie->set().isEmpty())
            sets.append(movie->set());
        if (!certifications.contains(movie->certification()) && !movie->certification().isEmpty())
            certifications.append(movie->certification());
    }
    ui->certification->addItems(certifications);
    ui->set->addItems(sets);

    ui->certification->setCurrentIndex(certifications.indexOf(m_movie->certification()));
    ui->set->setCurrentIndex(sets.indexOf(m_movie->set()));

    foreach (const Actor &actor, m_movie->actors()) {
        int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor.name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor.role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor.thumb);
    }
    foreach (const QString &genre, m_movie->genres()) {
        int row = ui->genres->rowCount();
        ui->genres->insertRow(row);
        ui->genres->setItem(row, 0, new QTableWidgetItem(genre));
    }
    foreach (const QString &studio, m_movie->studios()) {
        int row = ui->studios->rowCount();
        ui->studios->insertRow(row);
        ui->studios->setItem(row, 0, new QTableWidgetItem(studio));
    }
    foreach (const QString &country, m_movie->countries()) {
        int row = ui->countries->rowCount();
        ui->countries->insertRow(row);
        ui->countries->setItem(row, 0, new QTableWidgetItem(country));
    }

    if (!m_movie->posterImage()->isNull()) {
        ui->poster->setPixmap(QPixmap::fromImage(*m_movie->posterImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(m_movie->posterImage()->width()).arg(m_movie->posterImage()->height()));
    } else {
        ui->poster->setPixmap(QPixmap(":/img/film_reel.png"));
        ui->posterResolution->setText("");
    }

    if (!m_movie->backdropImage()->isNull()) {
        ui->backdrop->setPixmap(QPixmap::fromImage(*m_movie->backdropImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(m_movie->backdropImage()->width()).arg(m_movie->backdropImage()->height()));
    } else {
        ui->backdrop->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText("");
    }

    emit setActionSaveEnabled(true);
}

void MovieWidget::chooseMoviePoster()
{
    if (m_movie == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypePoster);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_movie->posters());
    MovieImageDialog::instance()->exec();

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false);
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = MovieImageDialog::instance()->imageUrl();
        m_posterDownloadManager->addDownload(d);
        ui->poster->setPixmap(QPixmap());
        ui->poster->setMovie(m_loadingMovie);
        m_hasChanged = true;
    }
}

void MovieWidget::chooseMovieBackdrop()
{
    if (m_movie == 0)
        return;

    MovieImageDialog::instance()->setImageType(TypeBackdrop);
    MovieImageDialog::instance()->clear();
    MovieImageDialog::instance()->setDownloads(m_movie->backdrops());
    MovieImageDialog::instance()->exec();

    if (MovieImageDialog::instance()->result() == QDialog::Accepted) {
        emit setActionSaveEnabled(false);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = MovieImageDialog::instance()->imageUrl();
        m_posterDownloadManager->addDownload(d);
        ui->backdrop->setPixmap(QPixmap());
        ui->backdrop->setMovie(m_loadingMovie);
        m_hasChanged = true;
    }
}

void MovieWidget::posterDownloadProgress(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster) {
        // ui->posterProgressBar->setRange(0, elem.bytesTotal);
        // ui->posterProgressBar->setValue(elem.bytesReceived);
    } else if (elem.imageType == TypeBackdrop) {
        // ui->backdropProgressBar->setRange(0, elem.bytesTotal);
        // ui->backdropProgressBar->setValue(elem.bytesReceived);
    }
}

void MovieWidget::posterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster) {
        m_chosenPoster = elem.image;
        ui->poster->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->posterResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
    } else if (elem.imageType == TypeBackdrop) {
        m_chosenBackdrop = elem.image;
        ui->backdrop->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->backdropResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0)
        emit setActionSaveEnabled(true);

}

void MovieWidget::saveInformation()
{
    this->setDisabledTrue();
    m_savingWidget->show();
    m_movie->setName(ui->name->text());
    m_movie->setOriginalName(ui->originalName->text());
    m_movie->setTagline(ui->tagline->text());
    m_movie->setRating(ui->rating->text().toFloat());
    m_movie->setReleased(ui->released->date());
    m_movie->setRuntime(ui->runtime->text().toInt());
    m_movie->setTrailer(QUrl(ui->trailer->text()));
    m_movie->setPlayCount(ui->playcount->text().toInt());
    m_movie->setLastPlayed(ui->lastPlayed->dateTime());
    m_movie->setOverview(ui->overview->toPlainText());
    m_movie->setCertification(ui->certification->currentText());
    m_movie->setSet(ui->set->currentText());
    m_movie->setWatched(ui->watched->isChecked());

    QList<Actor> actors;
    for (int i=0, n=ui->actors->rowCount() ; i<n ; i++) {
        Actor a;
        a.name = ui->actors->item(i, 0)->text();
        a.role = ui->actors->item(i, 1)->text();
        a.thumb = ui->actors->item(i, 0)->data(Qt::UserRole).toString();
        actors.append(a);
    }
    m_movie->setActors(actors);

    QStringList genres;
    for (int i=0, n=ui->genres->rowCount() ; i<n ; i++)
        genres.append(ui->genres->item(i, 0)->text());
    m_movie->setGenres(genres);

    QStringList studios;
    for (int i=0, n=ui->studios->rowCount(); i<n ; i++)
        studios.append(ui->studios->item(i, 0)->text());
    m_movie->setStudios(studios);

    QStringList countries;
    for (int i=0, n=ui->countries->rowCount() ; i<n ; i++)
        countries.append(ui->countries->item(i, 0)->text());
    m_movie->setCountries(countries);

    if (!m_chosenPoster.isNull())
        m_movie->setPosterImage(m_chosenPoster);
    if (!m_chosenBackdrop.isNull())
        m_movie->setBackdropImage(m_chosenBackdrop);

    if (m_loadedFromScraper) {
        QList<DownloadManagerElement> downloads;
        QList<Actor*> actors = m_movie->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; i++) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeActor;
            d.url = QUrl(actors.at(i)->thumb);
            d.actor = actors.at(i);
            downloads.append(d);
        }
        m_posterDownloadManager->setDownloads(downloads);
        emit actorDownloadStarted(tr("Downloading Missing Actor Images..."), m_progressMessageId);
        connect(m_posterDownloadManager, SIGNAL(allDownloadsFinished()), this, SLOT(downloadActorsFinished()), Qt::UniqueConnection);
    } else {
        this->downloadActorsFinished();
    }
}

void MovieWidget::downloadActorsFinished()
{
    emit actorDownloadFinished(m_progressMessageId);
    m_movie->saveData(Manager::instance()->mediaCenterInterface());
    this->setEnabledTrue();
    m_savingWidget->hide();
    m_hasChanged = false;
}

void MovieWidget::actorDownloadsLeft(int left)
{
    emit actorDownloadProgress(m_movie->actors().size()-left, m_movie->actors().size(), m_progressMessageId);
}

void MovieWidget::markHasChanged()
{
    m_hasChanged = true;
}

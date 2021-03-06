#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTableWidgetItem>
#include <QUrl>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QResizeEvent>

#include "data/ImageProviderInterface.h"
#include "data/Concert.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"
#include "smallWidgets/ImageLabel.h"

namespace Ui {
class ImageDialog;
}

/**
 * @brief The ImageDialog class
 * Displays a set of images
 */
class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageDialog(QWidget *parent = 0);
    ~ImageDialog();
    static ImageDialog *instance(QWidget *parent = 0);
    void setDownloads(QList<Poster> downloads, bool initial = true);
    QUrl imageUrl();
    void setImageType(ImageType type);
    void setItemType(ItemType type);
    void setMovie(Movie *movie);
    void setConcert(Concert *concert);
    void setTvShow(TvShow *show);
    void setSeason(int season);
    void setTvShowEpisode(TvShowEpisode *episode);
    void clear();
    void cancelDownloads();

public slots:
    void accept();
    void reject();
    int exec(int type);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void downloadFinished();
    void startNextDownload();
    void imageClicked(int row, int col);
    void chooseLocalImage();
    void onImageDropped(QUrl url);
    void onPreviewSizeChange(int value);
    void onZoomIn();
    void onZoomOut();
    void onSearch(bool onlyFirstResult = false);
    void onProviderChanged(int index);
    void onSearchFinished(QList<ScraperSearchResult> results);
    void onResultClicked(QTableWidgetItem *item);
    void onProviderImagesLoaded(QList<Poster> images);

private:
    Ui::ImageDialog *ui;

    /**
     * @brief The DownloadElement struct
     */
    struct DownloadElement {
        QUrl thumbUrl;
        QUrl originalUrl;
        QPixmap pixmap;
        QPixmap scaledPixmap;
        bool downloaded;
        ImageLabel *cellWidget;
        QSize resolution;
    };

    QNetworkAccessManager m_qnam;
    int m_currentDownloadIndex;
    QNetworkReply *m_currentDownloadReply;
    ImageType m_imageType;
    QList<DownloadElement> m_elements;
    QUrl m_imageUrl;
    int m_type;
    QList<ImageProviderInterface*> m_providers;
    Concert *m_concert;
    Movie *m_movie;
    TvShow *m_tvShow;
    TvShowEpisode *m_tvShowEpisode;
    ItemType m_itemType;
    QList<Poster> m_defaultElements;
    ImageProviderInterface *m_currentProvider;
    int m_season;
    int m_episode;

    QNetworkAccessManager *qnam();
    void renderTable();
    int calcColumnCount();
    int getColumnWidth();
    void loadImagesFromProvider(QString id);
};

#endif // IMAGEDIALOG_H

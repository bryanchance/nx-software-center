//
// Created by alexis on 23/01/18.
//

#include <QList>
#include "Updater.h"
#include <QDebug>

Updater::Updater(Repository *repository, const QList<Source *> &sources, QObject *parent)
        : QObject(parent), repository(repository), sources(sources),
          isWorking(false), isReady(false), fetchApplicationsInteractor(nullptr), executor(nullptr) {

}

bool Updater::isUpdateRequired() {
    const QDateTime &reference = QDateTime::currentDateTime().addDays(-1);
    return reference > lastUpdateDate;
}

QStringList Updater::getErrors() {
    return sourceErrors;
}

void Updater::update() {
    if (!isWorking) {
        setIsWorking();
        sourceErrors.clear();

        fetchApplicationsInteractor = new FetchApplicationsInteractor(sources);
        connect(fetchApplicationsInteractor, &FetchApplicationsInteractor::completed, this,
                &Updater::handleFetchApplicationsCompleted, Qt::QueuedConnection);

        fetchApplicationsInteractor->setAutoDelete(false);
        if (executor)
            executor->execute(fetchApplicationsInteractor);
        else
            fetchApplicationsInteractor->execute();
    }
    emit isWorkingChanged(isWorking);
}

void Updater::setIsWorking() {
    isWorking = true;
    emit isWorkingChanged(isWorking);
}

void Updater::unsetIsWorking() {
    isWorking = false;
    emit isWorkingChanged(false);
}

void Updater::handleFetchApplicationsCompleted() {
    if (!fetchApplicationsInteractor)
        return;

    const QList<Application> &applications = fetchApplicationsInteractor->getResults();
    sourceErrors = fetchApplicationsInteractor->getErrors();

    if (!applications.isEmpty()) {
        unsetIsReady();
        repository->setIsBeingUpdated(true);
        repository->removeAll();
        for (const Application &a: applications)
            repository->add(a);

        repository->setIsBeingUpdated(false);
        setIsReady();
    }

    unsetIsWorking();

    fetchApplicationsInteractor->deleteLater();
    fetchApplicationsInteractor = nullptr;
}

void Updater::setIsReady() {
    isReady = true;
    emit isReadyChanged(isReady);
}

void Updater::unsetIsReady() {
    isReady = false;
    emit isReadyChanged(isReady);
}

bool Updater::getIsWorking() { return isWorking; }

bool Updater::getIsReady() { return isReady; }

void Updater::setExecutor(Executor *executor) {
    Updater::executor = executor;
}



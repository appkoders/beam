package com.mw.beam.beamwallet.main

import com.mw.beam.beamwallet.baseScreen.BasePresenter

/**
 * Created by vain onnellinen on 10/4/18.
 */
class MainPresenter(currentView: MainContract.View, private val model: MainModel)
    : BasePresenter<MainContract.View>(currentView),
        MainContract.Presenter {

    override fun viewIsReady() {
        view?.configNavDrawer()
    }
}

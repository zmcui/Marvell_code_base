        /**
         * Default implementation of {@link #applySettings(CameraSettings)}
         * that is only missing the set of states it needs to wait for
         * before applying the settings.
         *
         * @param settings The settings to use on the device.
         * @param statesToAwait Bitwise OR of the required camera states.
         * @return Whether the settings can be applied.
         */
        protected boolean applySettingsHelper(CameraSettings settings,
                                              final int statesToAwait) {
            if (settings == null) {
                Log.v(TAG, "null argument in applySettings()");
                return false;
            }
            if (!getCapabilities().supports(settings)) {
                Log.w(TAG, "Unsupported settings in applySettings()");
                return false;
            }

            final CameraSettings copyOfSettings = settings.copy();
            try {
                getDispatchThread().runJob(new Runnable() {
                    @Override
                    public void run() {
                        CameraStateHolder cameraState = getCameraState();
                        // Don't bother to wait since camera is in bad state.
                        if (cameraState.isInvalid()) {
                            return;
                        }
                        cameraState.waitForStates(statesToAwait);
                        Log.w(TAG, "cuizm== settings in applySettings()");
                        getCameraHandler().obtainMessage(CameraActions.APPLY_SETTINGS, copyOfSettings)
                                .sendToTarget();
                    }});
            } catch (final RuntimeException ex) {
                getAgent().getCameraExceptionHandler().onDispatchThreadException(ex);
            }
            return true;
        }

